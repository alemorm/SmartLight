#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "SPIFFS.h"
#include "time.h"

/* Debug flag to print useful output, set to 0 for final deployment */
const int debugVar = 1;

/* Key Authentication Parameters*/
// File that contains the authentication information for WiFI and IFTTT
const char* keyFile = "/auth.keys";
// Line width limit for auth.keys file
const int lineWidth = 512;

/* WiFi info */
// Name of 2.4GHz wireless network, read from "auth.keys" for security reasons
char wifiName[lineWidth];
// WiFi password, read from "auth.keys" for security reasons
char wifiKey[lineWidth];

/* IFTTT Authentication */
// First part of the IFTTT webhook URL
const char* iftttURL = "https://maker.ifttt.com/trigger";
// Unique IFTTT key, read from "auth.keys" for security reasons
char iftttKey[lineWidth];

/* Pinout parameters */
// ADC1 input-only pin independent of WiFi
const int sensorPin = 34;
// ADC1 input-output pin independent of WiFi
const int ledPin = 32;

/* Sampling parameters */
// Delay between each light sampling in milliseconds
int readDelay = 500;
// Number of light samples per average
int sampleNum = 300; /*NOTE* 1,000 or more integer array elements cause stack overflow on ESP32 */
// Light intensity threshold to turn lamp off, it should be calibrated to desired brightness
int lightIntensity = 325;

/* Time Server Information */
// Google's public NTP server
const char* ntpServer = "time.google.com"; 
// GMT offset in seconds (PST = GMT-8 = -8*3600)
const long gmtOffset = -28800;
// daylight saving offset in seconds (PDT = GMT-7 = 3600)
const int daylightOffset = 3600;

/* Finite state variables to prevent redundant triggers */
// Variable that records last time the lamp was turned on
int checkonPrevious = 1;

/* Function Initializations */
// Function that returns time and date info from NTP server
void localTime(char *timeBuffer);
// Function that retrieves the authentication keys from the auth.keys file
void getKeys(char *wName, char *wKey, char *iKey); 
// Function that sends the HTTP POST request to IFTTT to triggger services
void httpQuery(const char *eventType);
// Function that concatenates the HTTP URL before sending POST request
void httpConcat(char *httpBuffer, const int bufferLength, const char *mURL, const char *eType, const char *iKey); 

void setup(){  
  // Define baud rate for serial communication
  Serial.begin(9600);
  
  // Retrieve keys from auth.keys file
  getKeys(wifiName, wifiKey, iftttKey);

  // Test LED code for debugging
  if (debugVar) {
    // Set up the LED pin to be an output.
    pinMode(ledPin, OUTPUT);
  }

  // Set 10-bit ADC resolution
  analogReadResolution(10);

  // Delay before connecting to WiFi
  delay(4000);
  // Connect to WiFi
  WiFi.begin(wifiName, wifiKey);
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
    
  // Initialize the NTP server protocol
  configTime(gmtOffset, daylightOffset, ntpServer);
}

void loop() {
  int lightSamples[sampleNum], hourVar;
  double lightAverage = 0;
  char timeHour[3];
  
  // Collect sampleNum number of samples to average
  for (int i=0; i<sampleNum; i++) {
    // Read the current light levels
    lightSamples[i] = analogRead(sensorPin);
    lightAverage += lightSamples[i];
    delay(readDelay);
  }

  // Calculate the average value of the light samples
  lightAverage /= sampleNum;

  if (debugVar) {
  // Print Light Values
  Serial.print("Light Value = ");
  Serial.println(lightAverage);
  }

  // Get current hour from NTP server
  localTime(timeHour);
  // Convert hour value from char to int
  hourVar = atoi(timeHour);

  // Turn off lamp from 11:00pm to 9:00am
  if ((hourVar >= 23 || hourVar <= 9)) {
    // Check if lamp is off, and if not, turn it off
    if (!checkonPrevious) {
      if (debugVar) {
        // Turn off LED
        digitalWrite(ledPin, LOW);
      }
      checkonPrevious = 1;
      httpQuery("lamp_off");
    }
  }
  // Turn on lamp when light intensity falls below a threshold and is not on
  else if (lightAverage <= lightIntensity && checkonPrevious) {
    if (debugVar) {
      // Turn on LED
      digitalWrite(ledPin, HIGH);
    }
    checkonPrevious = 0;
    httpQuery("lamp_on");
  }
}

// Function that returns time and date info from NTP server
void localTime(char *timeBuffer){
  // Initialize the time structure variable
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }
  
  if (debugVar) {
    // Print the complete time and date from NTP server
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }
  // Retrieve the hour field from the time structure
  strftime(timeBuffer,3, "%H", &timeinfo);
}

// Function that retrieves the authentication keys from the auth.keys file
void getKeys(char *wName, char *wKey, char *iKey) {
  // File descriptor for auth.keys
  File fileDescriptor;
  // Match variable for the double quotes enclosing each key
  char doubleQuote = '"';
  // Match counter for the number of keys enclosed in double quotes
  int matchCount = 0;  

  if (debugVar) {
    // Check if the Serial Peripheral Interface Flash File System (SPIFFS) was mounted
    if (!SPIFFS.begin(true)) {
      Serial.println("An Error has occurred while mounting SPIFFS");
    }
  }

  // Open key authentication files
  fileDescriptor = SPIFFS.open(keyFile, "r");

  // Continually read bytes in the auth.keys file
  while (fileDescriptor.available()) {
    // Read 1 character in the auth.keys file
    char readChar = fileDescriptor.read();
    // When the first double quote is encountered we have the first key
    if (readChar == doubleQuote) {
      matchCount++;
    } else {
      // Odd values for matchCount represent a new key value
      /* Key strings are constructed by dereferencing the char array pointer
      and post-incrementing the pointer address after assignment */
      if (matchCount == 1) {
        *(wName++) = readChar;
      } else if (matchCount == 3) {
        *(wKey++) = readChar;
      } else if (matchCount == 5) {
        *(iKey++) = readChar;
      }
    }
  }
}

// Function that sends the HTTP POST request to IFTTT to triggger services
void httpQuery(const char *eventType) {
  // Initialize the HTTP structure variable
  HTTPClient http;
  // Initialize the URL string size variable
  const int strSize = 100;
  // Initialize the URL string variable
  char httpURL[strSize];
  // Initialize the HTTP response code variable
  int httpResponseCode = 0;
  // Set all the bytes in the created array to zero
  memset(httpURL, 0, strSize);
  // Create the URL from the components
  httpConcat(httpURL, strSize, iftttURL, eventType, iftttKey);
  // Specify destination for HTTP request
  http.begin(httpURL);
  // Specify content-type header
  http.addHeader("Content-Type", "text/plain");
  // Send the actual POST request until it succeeds
  while (httpResponseCode <= 0) {
    httpResponseCode = http.POST("POSTING from ESP32");
    if (debugVar) {
      // Print the URL for debugging
      Serial.print("URL = ");
      Serial.println(httpURL);
      // Get the response to the request
      String response = http.getString();
      Serial.print("HTTP POST response code: ");
      // Print return code
      Serial.println(httpResponseCode);
    }
    // Small delay before sending each POST request to balance the server load
    delay(readDelay);
  }
}

// Function that concatenates the HTTP URL before sending POST request
void httpConcat(char *httpBuffer, const int bufferLength, const char *mURL, const char *eType, const char *iKey) {
    // Format copy the three strings into the input string buffer
    snprintf(httpBuffer, bufferLength, "%s/%s/%s", mURL, eType, iKey);
}
