/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/
*********/

#include <Arduino.h>

#define LED 2

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  // Set LED pin as output
  pinMode(LED, OUTPUT);
}

void loop() {
  // Turn LED on
  digitalWrite(LED, HIGH);
  Serial.println("LED is on");
  delay(1000);
  // Turn LED off
  digitalWrite(LED, LOW);
  Serial.println("LED is off");
  delay(1000);
}   