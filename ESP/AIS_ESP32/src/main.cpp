/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/
*********/

#include <Arduino.h>

#define LED 2

// ---- PINS ----
// GPIO 32-35: Moisture Sensors Input
#define MSI_0 32
// GPIO 16-19: Pump Control Output
#define PCO_0 16

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  // Set LED pin as output
  pinMode(LED, OUTPUT);
  pinMode(MSI_0, INPUT);
  pinMode(PCO_0, OUTPUT);
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

  uint16_t anal_input = analogRead(MSI_0);
  Serial.print("Anal_Input: ");
  Serial.println(anal_input);
  delay(1000);

  digitalWrite(PCO_0, HIGH);
  delay(1000);
  digitalWrite(PCO_0, LOW);
  delay(1000);

}   