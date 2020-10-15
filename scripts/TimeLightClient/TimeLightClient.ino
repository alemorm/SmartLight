#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"

/* Debug flag to print useful output, set to 0 for final deployment */
const int debugVar = 1;

/* WiFi info */
// Name of 2.4GHz wireless network, not included for security reasons
const char* wifiName =
// WiFi password, not included for security reasons
const char* wifiKey =

/* IFTTT Authentication */
// First part of the IFTTT webhook URL
const char* iftttURL = "https://maker.ifttt.com/trigger";
// Unique IFTTT key, not included for security reasons
const char* iftttKey =

/* Pinout parameters */
// ADC1 input-only pin independent of WiFi
const int sensorPin = 34;
// ADC1 input-output pin independent of WiFi
const int ledPin = 32;

/* Sampling parameters */
// Delay between each light sampling in milliseconds
int readDelay = 500;
// Number of light samples per average
int sampleNum = 600; /*NOTE* 1,000 or more integer array elements cause stack overflow on ESP32 */
// Light intensity threshold to turn lamp off, it should be calibrated
int lightIntensity = 300;

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
// Variable that records last time the lamp was turned off
int checkoffPrevious = 1;

/* Function Initializations */
// Function that returns time and date info from NTP server
void returnLocalTime(char timeBuffer[3]);
// Function that sends the HTTP POST request to IFTTT to triggger services
void httpQuery(const char *eventType);
// Function that concatenates the HTTP URL before sending POST request
void httpConcat(char *httpBuffer, const int bufferLength, const char *s1, const char *s2, const char *s3); 

void setup(){  
  // Define baud rate for serial communication
  Serial.begin(9600);

  // Test LED code for debugging
  if (debugVar) {
    // Set up the LED pin to be an output.
    pinMode(ledPin, OUTPUT);
  }

  // Set 10-bit ADC resolution
  analogReadResolution(10);

  // Delay before connecting to WiFi
  delay(4000);
  WiFi.begin(wifiName, wifiKey);
  if (debugVar) {
    while (WiFi.status() != WL_CONNECTED) { //Check for the connection
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
  }
  
  // Initialize the NTP server protocol
  configTime(gmtOffset, daylightOffset, ntpServer);
}

void loop() {
  int lightSamples[sampleNum], hourVar;
  double lightAverage = 0;
  char timeHour[3];
  
  // Collect sampleNum number of samples to averageS
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
  returnLocalTime(timeHour);
  // Convert hour value from char to int
  hourVar = atoi(timeHour);

  // Turn off lamp from 11:00pm to 8:00am
  if ((hourVar >= 23 || hourVar <= 8)) {
    // Check if lamp is off, and if not, turn it off
    if (checkoffPrevious) {
      if (debugVar) {
        // Turn off LED
        digitalWrite (ledPin, LOW);
      }
      checkonPrevious = 1;
      checkoffPrevious = 0;
      httpQuery("lamp_off");
    }
  }
  else {
      // Turn off lamp when light intensity surpasses a threshold and is not off
      if (lightAverage > lightIntensity && checkoffPrevious) {
        if (debugVar) {
          // Turn off LED
          digitalWrite (ledPin, LOW);
        }
        checkonPrevious = 1;
        checkoffPrevious = 0;
        httpQuery("lamp_off");
      }
      // Turn on lamp when light intensity falls below a threshold and is not on
      else if (lightAverage <= lightIntensity && checkonPrevious) {
        if (debugVar) {
          // Turn on LED
          digitalWrite (ledPin, HIGH);
        }
        checkonPrevious = 0;
        checkoffPrevious = 1;
        httpQuery("lamp_on");
      }   
  }
}

// Function that returns time and date info from NTP server
void returnLocalTime(char timeBuffer[3]){
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

// Function that sends the HTTP POST request to IFTTT to triggger services
void httpQuery(const char *eventType) {
    // Initialize the HTTP structure variable
    HTTPClient http;
    // Initialize the URL string size variable
    const int strSize = 100;
    // Initialize the URL string variable
    char httpURL[strSize];
    // Set all the bytes in the created array to zero
    memset(httpURL, 0, strSize);
    // Create the URL from the components
    httpConcat(httpURL, strSize, iftttURL, eventType, iftttKey);
    // Specify destination for HTTP request
    http.begin(httpURL);
    // Specify content-type header
    http.addHeader("Content-Type", "text/plain");
    // Send the actual POST request
    int httpResponseCode = http.POST("POSTING from ESP32");

    if (debugVar) {
      // Print the URL for debugging
      Serial.print("URL = ");
      Serial.println(httpURL);
      if (httpResponseCode > 0){
        // Get the response to the request
        String response = http.getString();
        // Print return code
        Serial.println(httpResponseCode);
        // Print request response
        Serial.println(response);
      } else {
        Serial.print("Error on sending POST: ");
        // Print return code
        Serial.println(httpResponseCode);
      }
    }
}

// Function that concatenates the HTTP URL before sending POST request
void httpConcat(char *httpBuffer, const int bufferLength, const char *s1, const char *s2, const char *s3) {
    // Format copy the three strings into the input string buffer
    snprintf(httpBuffer, bufferLength, "%s/%s/%s", s1, s2, s3);
}
