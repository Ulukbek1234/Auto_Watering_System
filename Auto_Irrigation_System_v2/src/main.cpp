#include <Arduino.h>

#include "Zone.h"
#include "Config.h"

Zone *pots;

void setup() {
  Serial.begin(9600);
  pots = new Zone();
  //Serial1.begin(115200); // Esp 32

  pots->addPump(8, 0.7);
  pots->addPump(9, 0.7);
  // pots->addPump(10, 5);
  // pots->addPump(11, 5);

  // pots->addSoilSensor(A0);
  // pots->addSoilSensor(A1);
  // pots->addSoilSensor(A2);
  // pots->addSoilSensor(A3);
}

void loop() {
  delay(1000 * 10); // seconds
  
  pots->startAutoIrrigating();
  pots->updateDay();
  Serial.println(pots->getData());
  DEBUG_PRINTLN("---------------------");
}
