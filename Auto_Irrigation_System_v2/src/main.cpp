#include <Arduino.h>

#include "Zone.h"
#include "Config.h"

Zone *pots;

void setup() {
  pots = new Zone();
  Serial.begin(9600);
  Serial1.begin(115200); // Esp 32
  delay(1000);

  pots->addPump(8, 5);
  pots->addSoilSensor(A0);
}

void loop() {
  delay(1000 * 10);
  pots->startAutoIrrigating();
  pots->updateDay();
  Serial.println(pots->getData());
}
