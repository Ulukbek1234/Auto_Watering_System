#include <Arduino.h>

#include "Zone.h"
#include "Config.h"
#include "Utils.h"

Zone *pots;
Zone *auto_pot;
Zone all_pots[2]; // TODO dynamic array if more pots needed in future
unsigned long start_time = 0;
const unsigned long wait_time = 60000; // 1 minute

void setup() {
  Serial.begin(9600);
  pots = new Zone(0);
  pots->setOperationMode(MODE_SOIL);

  pots->addPump(8, 0.5);
  pots->addPump(9, 0.5);
  pots->addPump(10, 0.2);
  
  pots->addSoilSensor(A0);
  pots->addSoilSensor(A1);
  pots->addSoilSensor(A2);
  // pots->addWaterLevelSensor(A0);
  
  auto_pot = new Zone(1);
  auto_pot->setOperationMode(MODE_MANUAL);
  auto_pot->addPump(11, 0.4);
  auto_pot->addSoilSensor(A3);

  all_pots[0] = *pots;
  all_pots[1] = *auto_pot;
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

void commandHandler(String serial_input) {
    String command = "";
    bool real_command = Utils::findDataFromMessage(serial_input, "CMD:", command);
    if(!real_command)
    {
      DEBUG_PRINT("No command found: ");
      DEBUG_PRINTLN(serial_input);
      return;
    } 
    int zone = Utils::findDataFromMessage(serial_input, "ZONE:", command) ? command.toInt() : -1;
    if(zone < 0 || zone >= 2) {
      Serial.println("Invalid zone specified");
      return;
    }


    if (command == "TELEM") {
      Serial.println(all_pots[zone].getData());
      DEBUG_PRINTLN("Sent telemetry data.");
    } else if (command == "SET_MODE") {
      // TODO which zone selected?
      String mode = "";
      if(Utils::findDataFromMessage(serial_input, "NEW_MODE:", mode))
      {
        all_pots[zone].setOperationMode(static_cast<OperationModes>(mode.toInt()));
        Serial.print("NEW_MODE_SELECTED: ");
        Serial.println(mode);
        DEBUG_PRINTLN("Set operation mode to: " + mode);
      } else {
        DEBUG_PRINTLN("Wrong operation mode");
      }
    } else if (command == "MAN_IRR") {
      DEBUG_PRINTLN("Started manual irrigating.");
      String pump_id = "";
      String amount = "";
      Utils::findDataFromMessage(serial_input, "PUMP:", pump_id);
      Utils::findDataFromMessage(serial_input, "AMOUNT:", amount);
      all_pots[zone].manualIrrigation(pump_id.toInt(), amount.toFloat());
      Serial.println("MANUAL_IRRIGATION_DONE");
    } else if (command == "SAV_EEP") {
      all_pots[zone].saveToEEPROM();
      Serial.println("SAVING_TO_EEPROM");
    }
    else {
      DEBUG_PRINT("Unknown command");
      DEBUG_PRINTLN(command);
    }

}

void loop() {
  // Check serial for commands from master here
  if(Serial.available() > 0) {
    String serial_input = Serial.readStringUntil('\n');
    DEBUG_PRINTLN("Received command: " + serial_input);
    serial_input.toUpperCase();
    serial_input.trim();
    commandHandler(serial_input);
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
