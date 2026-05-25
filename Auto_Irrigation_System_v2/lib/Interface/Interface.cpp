#include "Interface.h"

Interface::Interface(EE_Data_t eeprom_data_param) : comms(), zone(0)
{
  eeprom_data = new EE_Data_t(eeprom_data_param);
  
  zone.addPump(PUMP_0, 0.5);
  zone.addPump(PUMP_1, 0.5);
  zone.addPump(PUMP_2, 0.5);
  zone.addPump(PUMP_3, 0.5);
  zone.addSoilSensor(HUM_SNS_0, eeprom_data->cali_air[0], eeprom_data->cali_water[0]);
  zone.addSoilSensor(HUM_SNS_1, eeprom_data->cali_air[1], eeprom_data->cali_water[1]);
  zone.addSoilSensor(HUM_SNS_2, eeprom_data->cali_air[2], eeprom_data->cali_water[2]);
  zone.addSoilSensor(HUM_SNS_3, eeprom_data->cali_air[3], eeprom_data->cali_water[3]);

}

/*
  Performs commands sent from Serial
  Response: CMD: RESP, STATUS: (SUCC, FAIL)
*/

void Interface::commandHandler(String input, COMMS_TYPE type) {
    bool status = false;
    String response = "CMD:RESP,STATUS:";

    String command = "";
    bool real_command = Utils::findDataFromMessage(input, "CMD:", command);
    if(!real_command)
    {
      DEBUG_PRINT("No command found: ");
      DEBUG_PRINTLN(input);
      comms.write(response + "FAIL", type);
      return;
    } 

    if (command == "TELEM") {
      comms.write(zone.getData(), type);
      DEBUG_PRINTLN("Sent telemetry data.");
      return;
    } 

    if (command == "SAV_EEP")
    {
      zone.saveToEEPROM(eeprom_data);
      Preferences prefs;
      prefs.begin("EE_Data");
      // Store the struct as bytes
      prefs.putBytes("EE_Data", &eeprom_data, sizeof(eeprom_data));
      prefs.end();
      status = true;
      return;
    }

    if (command == "RST_EEP")
    {
      // for(int i = 0; i < nr_zones; i++)
      // {
      //   all_pots[i]->resetEEPROM();
      // }
      status = false;
      return;
    }
    
    if (command == "SET_MODE") {
      String mode = "";
      String pump_id = "";
      bool found_mode = Utils::findDataFromMessage(input, "NEW_MODE:", mode); 
      bool found_pump = Utils::findDataFromMessage(input, "PUMP:", pump_id);
      if(found_mode && found_pump)
      {
        zone.setOperationMode(static_cast<OperationModes>(mode.toInt()), pump_id.toInt());
        status = true;
        DEBUG_PRINTLN("Set operation mode to: " + mode);
      } else {
        DEBUG_PRINTLN("Failed to NEW_MODE: Wrong operation mode");
        status = false;
      }
    } else if (command == "MAN_IRR") {
      DEBUG_PRINTLN("Started manual irrigating.");
      String pump_id = "";
      String amount = "";
      bool found_pump = Utils::findDataFromMessage(input, "PUMP:", pump_id);
      bool found_amount = Utils::findDataFromMessage(input, "AMOUNT:", amount);
      if(found_pump && found_amount)
      {
        zone.manualIrrigation(pump_id.toInt(), amount.toFloat());
        status = true;
      } else {
        DEBUG_PRINTLN("Failed to MAN_IRR: find pump or amount");
        status = false;
      }
    } else if (command == "CHG_DLY_LTR") {
      String pump_id = "";
      String new_limit = "";
      bool found_pump = Utils::findDataFromMessage(input, "PUMP:", pump_id);
      bool found_limit = Utils::findDataFromMessage(input, "NEW_LIM:", new_limit);
      if(found_pump && found_limit)
      {
        zone.changeDailyLimit(pump_id.toInt(), new_limit.toFloat());
        DEBUG_PRINTLN("Changed daily limit to: " + new_limit);
        status = true;
      } else {
        DEBUG_PRINTLN("Failed to CHG_DLY_LTR: find pump or limit");
        status = false;
      }
    } else if (command == "CHG_MOI_THR") {
      String new_limit = "";
      bool found_limit = Utils::findDataFromMessage(input, "NEW_LIM:", new_limit);
      if(found_limit)
      {
        zone.changeMoistureThreshold(new_limit.toInt());
        status = true;
      } else {
        DEBUG_PRINTLN("Failed to CHG_MOI_THR: find limit");
        status = false;
      }
    } else if (command == "CALI_AIR") {
      String soil_pin = "";
      Utils::findDataFromMessage(input, "SOIL_PIN:", soil_pin);
      status = zone.caliSoilInAir(soil_pin.toInt());
    } else if (command == "CALI_WATER") {
      String soil_pin = "";
      Utils::findDataFromMessage(input, "SOIL_PIN:", soil_pin);
      status = zone.caliSoilInWater(soil_pin.toInt());
    } else {
      DEBUG_PRINT("Unknown command");
      DEBUG_PRINTLN(command);
      status = false;
    }
    comms.write(response + (status ? "SUCC" : "FAIL"), type);
}

void Interface::startAutoIrrigation() {
    zone.startAutoIrrigation();
}

void Interface::updateDay() {
    zone.updateDay();
}

void Interface::readCommand() {
    for(int i = 0; i < comms.nr_active_types; i++) {
        COMMS_TYPE type = comms.active_types[i];
        String command = comms.read(type);
        if(command != "")
        {
            DEBUG_PRINTLN("Received command: " + command);
            command.toUpperCase();
            command.trim();
            commandHandler(command, type);
        }
    }
}