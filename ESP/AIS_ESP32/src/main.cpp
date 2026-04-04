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
#define PUMP_0 16
#define PUMP_1 17
#define PUMP_2 18
#define PUMP_3 19


void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  // Set LED pin as output
  pinMode(PUMP_0, OUTPUT);
  pinMode(PUMP_1, OUTPUT);
  pinMode(PUMP_2, OUTPUT);
  pinMode(PUMP_3, OUTPUT);
  digitalWrite(PUMP_0, LOW);
  digitalWrite(PUMP_1, LOW);
  digitalWrite(PUMP_2, LOW);
  digitalWrite(PUMP_3, LOW);

}

void loop() {
  for(int i = 0; i < 4; i++)
  {
    Serial.print("Active Pump ");
    Serial.println(i);
    delay(1000);
    digitalWrite(i + 16, LOW);
    delay(1000);
    digitalWrite(i + 16, HIGH);
  }
  delay(5000);
}   