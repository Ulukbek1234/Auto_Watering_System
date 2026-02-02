#include <Arduino.h>

#include "Zone.h"
#include "Config.h"
#include "Utils.h"

Zone *pots;
unsigned long start_time = 0;
const unsigned long wait_time = 10000; //TODO remove after debugging //60000; // 1 minute

void setup() {
  Serial.begin(9600);
  pots = new Zone();
  pots->setOperationMode(MODE_MANUAL);

  pots->addPump(8, 0.3);
  pots->addPump(9, 0.3);
  pots->addPump(10, 0.1);
  // pots->addPump(11, 5);

  // pots->addWaterLevelSensor(A0);

  pots->addSoilSensor(A0);
  pots->addSoilSensor(A1);
  pots->addSoilSensor(A2);
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
    - Slave to master commands
*/

/*
cmd:man_irr,pump:9,amount:0.3

*/

void loop() {
  // Check serial for commands from master here
  if(Serial.available() > 0) {
    String serial_input = Serial.readStringUntil('\n');
    DEBUG_PRINTLN("Received command: " + serial_input);
    serial_input.toUpperCase();
    serial_input.trim();
    // TODO clean this up, what a mess
    
    String command = "";
    bool real_command = Utils::findDataFromMessage(serial_input, "CMD:", command);
    if(!real_command)
    {
      DEBUG_PRINT("No command found: ");
      DEBUG_PRINTLN(serial_input);
    } else if (command == "SYNCH") {
      pots->resetDayProgression();
      DEBUG_PRINTLN("Day progression reset.");
    } else if (command == "TELEM") {
      Serial.println(pots->getData());
      DEBUG_PRINTLN("Sent telemetry data.");
    } else if (command == "SET_MODE") {
      String mode = "";
      if(Utils::findDataFromMessage(serial_input, "NEW_MODE:", mode))
      {
        pots->setOperationMode(static_cast<OperationModes>(mode.toInt()));
        DEBUG_PRINTLN("Set operation mode to: " + mode);
      } else {
        DEBUG_PRINTLN("Wrong operation mode");
      }
    } else if (command == "MAN_IRR") {
      DEBUG_PRINTLN("Started manual irrigating.");
      String pump_id = "";
      String amount = "";
      // TODO checks if correct format
      Utils::findDataFromMessage(serial_input, "PUMP:", pump_id);
      Utils::findDataFromMessage(serial_input, "AMOUNT:", amount);
      pots->manualIrrigation(pump_id.toInt(), amount.toFloat());
      Serial.println("MANUAL_IRRIGATION_DONE");
    } 
    else {
      DEBUG_PRINT("Unknown command");
      DEBUG_PRINTLN(command);
    }
  }
  // Non-Blocking delay logic
  if(millis() - start_time >= wait_time)
  {
    start_time = millis();
    DEBUG_PRINTLN("Waiting period over, checking sensors again.");
    pots->startAutoIrrigating();
    pots->updateDay();
  }
}
