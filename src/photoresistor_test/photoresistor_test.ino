/**
 * Interfacing Photoresistor using ESP32
 * By TechMartian
 *
 */
// Sensor pin constants
const int sensorPin = 34;
const int ledPin = 32;

// Sampling parameters
int readDelay =  500;
int sampleRate = 12;
int printDelay = 6000;

void setup(){
  Serial.begin(9600);  /* Define baud rate for serial communication */
  // We'll set up the LED pin to be an output.
  pinMode(ledPin, OUTPUT);
  analogReadResolution(10);
}

void loop(){
  int lightSamples[sampleRate];
  double lightAverage = 0;
  
  // Collect sampleRate number of samples
  for (int i=0; i<sampleRate; i++) {
      lightSamples[i] = analogRead(sensorPin); // read the current light levels
      lightAverage += lightSamples[i];
      delay(readDelay);
  }

  // Calculate the average value of the light samples
  lightAverage /= sampleRate;
  
  // Print lightVal
  Serial.print("Light Value = ");
  Serial.println(lightAverage);
  
  if(lightAverage <  500) {
    digitalWrite (ledPin, HIGH); // turn on light
  }
  //otherwise, it is bright
  else {
    digitalWrite (ledPin, LOW); // turn off light
  }
  delay(printDelay);
}
