#include <Arduino.h>

#include "Zone.h"
#include "Config.h"

Zone *pots;
unsigned long start_time = 0;
const unsigned long wait_time = 60000; // 1 minute

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
  start_time = millis();
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
  // Check serial for commands from master here
  if(Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
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
    start_time = millis(); // Reset wait time on command received
  }
  
  pots->startAutoIrrigating();
  pots->updateDay();
   

  // Non-Blocking delay logic
  if(millis() - start_time >= wait_time)
  {
    start_time = millis();
    DEBUG_PRINTLN("Waiting period over, checking sensors again.");
  }
  DEBUG_PRINTLN("---------------------");
}
