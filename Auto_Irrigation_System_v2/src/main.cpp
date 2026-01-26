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


// TODO
/*
  - Commands from master
  - Different modes of irrigation
    - Max daily limit
    - Moisture threshold
    - Time based
*/
void loop() {
  delay(1000 * 60); // seconds
  // Check serial for commands from master here
  String command = Serial.readStringUntil('\n');
  if (command.length() > 0) {
    DEBUG_PRINTLN("Received command: " + command);
    command.trim();
    if (command == "SYNCH") {
      pots->resetDayProgression();
      DEBUG_PRINTLN("Day progression reset.");
    } else if (command == "TELEMETRY") {
      Serial.println(pots->getData());
      DEBUG_PRINTLN("Sent telemetry data.");
    }
    else {
      DEBUG_PRINTLN("Unknown command.");
    }
  }
  
  pots->startAutoIrrigating();
  pots->updateDay();
  DEBUG_PRINTLN("---------------------");
}
