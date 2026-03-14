#include <Arduino.h>

#include "Zone.h"
#include "Config.h"
#include "Utils.h"
#include "EEPROMex.h"

Zone *pots;
Zone *auto_pot;
int nr_zones = 1; // TODO change dynamically
Zone *all_pots[1]; // TODO dynamic array if more pots needed in future
unsigned long start_time = 0;
const unsigned long wait_time = 10000; // 1 minute
// EEPROM 
EE_Data eeprom_data;


void setup() {
  Serial.begin(9600);
  int offset = 0;
  pots = new Zone(0, offset);
  pots->setOperationMode(MODE_MANUAL);

  pots->addPump(8, 1.0);
  pots->addPump(9, 1.0);
  pots->addPump(10, 1.0);
  pots->addPump(11, 1.0);
  
  pots->addSoilSensor(A0);
  pots->addSoilSensor(A1);
  pots->addSoilSensor(A2);
  pots->addSoilSensor(A4);
  
  offset = pots->eeprom_offset_end;

  all_pots[0] = pots;
  
  EEPROM.setMaxAllowedWrites(100);
  EEPROM.readBlock(0, eeprom_data);

  start_time = millis();
}

/*
  Performs commands sent from Serial
  Response: CMD: RESP, STATUS: (SUCC, FAIL)
*/

void commandHandler(String serial_input) {
    bool status = false;
    String response = "CMD:RESP,STATUS:";

    String command = "";
    bool real_command = Utils::findDataFromMessage(serial_input, "CMD:", command);
    if(!real_command)
    {
      DEBUG_PRINT("No command found: ");
      DEBUG_PRINTLN(serial_input);
      Serial.println(response + "FAIL");
      return;
    } 

    if (command == "TELEM") {
      for(int i = 0; i < nr_zones; i++) {
        Serial.print(all_pots[i]->getData());
      }
      Serial.println("");
      DEBUG_PRINTLN("Sent telemetry data.");
      return;
    } 

    if (command == "SAV_EEP")
    {
      
      
      for(int i = 0; i < nr_zones; i++)
      {
        all_pots[i]->saveToEEPROM(&eeprom_data);
        Serial.print("eeprom valus: ");
        Serial.println(eeprom_data.cali_air[i]);
        Serial.println(eeprom_data.cali_water[i]);
        Serial.println(eeprom_data.total_liters[i]);
      }

      EEPROM.writeBlock(0, eeprom_data);
      status = false;
      return;
    }

    if (command == "RST_EEP")
    {
      // for(int i = 0; i < nr_zones; i++)
      // {
      //   all_pots[i]->resetEEPROM();
      // }
      // Serial.println("RESET_EEPROM");
      status = false;
      return;
    }
    
    String zone_string = "";
    int zone = Utils::findDataFromMessage(serial_input, "ZONE:", zone_string) ? zone_string.toInt() : -1;
    if(zone < 0 || zone >= nr_zones) {
      Serial.println(response + "FAIL");      
      return;
    }
    
    if (command == "SET_MODE") {
      // TODO which zone selected?
      String mode = "";
      if(Utils::findDataFromMessage(serial_input, "NEW_MODE:", mode))
      {
        all_pots[zone]->setOperationMode(static_cast<OperationModes>(mode.toInt()));
        status = true;
        DEBUG_PRINTLN("Set operation mode to: " + mode);
      } else {
        DEBUG_PRINTLN("Wrong operation mode");
        status = false;
      }
    } else if (command == "MAN_IRR") {
      DEBUG_PRINTLN("Started manual irrigating.");
      String pump_id = "";
      String amount = "";
      bool found_pump = Utils::findDataFromMessage(serial_input, "PUMP:", pump_id);
      bool found_amount = Utils::findDataFromMessage(serial_input, "AMOUNT:", amount);
      if(found_pump && found_amount)
      {
        all_pots[zone]->manualIrrigation(pump_id.toInt(), amount.toFloat());
        status = true;
      } else {
        DEBUG_PRINTLN("Failed to find pump or amount");
        status = false;
      }
    } else if (command == "CHG_DLY_LTR") {
      String pump_id = "";
      String new_limit = "";
      bool found_pump = Utils::findDataFromMessage(serial_input, "PUMP:", pump_id);
      bool found_limit = Utils::findDataFromMessage(serial_input, "NEW_LIM:", new_limit);
      if(found_pump && found_limit)
      {
        all_pots[zone]->changeDailyLimit(pump_id.toInt(), new_limit.toFloat());
        DEBUG_PRINTLN("Changed daily limit to: " + new_limit);
        status = true;
      } else {
        DEBUG_PRINTLN("Failed to find pump or limit");
        status = false;
      }
    } else if (command == "CALI_AIR") {
      String soil_pin = "";
      Utils::findDataFromMessage(serial_input, "SOIL_PIN:", soil_pin);
      status = all_pots[zone]->caliSoilInAir(soil_pin.toInt());
    } else if (command == "CALI_WATER") {
      String soil_pin = "";
      Utils::findDataFromMessage(serial_input, "SOIL_PIN:", soil_pin);
      status = all_pots[zone]->caliSoilInWater(soil_pin.toInt());
    } else {
      DEBUG_PRINT("Unknown command");
      DEBUG_PRINTLN(command);
      status = false;
    }
    Serial.println(response + (status ? "SUCC" : "FAIL"));
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
    for(int i = 0; i < nr_zones; i++)
    {
      all_pots[i]->startAutoIrrigating();
      all_pots[i]->updateDay();
    }
  }
}
