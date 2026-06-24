#include "Interface.h"

Interface::Interface(EE_Data_t eeprom_data_param) : comms(), zone(0)
{
  eeprom_data = new EE_Data_t(eeprom_data_param);
  
  // TODO probably use for loop for this
  zone.addPump(PUMP_0, eeprom_data->max_liters[0], eeprom_data->total_liters[0]);
  zone.addPump(PUMP_1, eeprom_data->max_liters[1], eeprom_data->total_liters[1]);
  zone.addPump(PUMP_2, eeprom_data->max_liters[2], eeprom_data->total_liters[2]);
  zone.addPump(PUMP_3, eeprom_data->max_liters[3], eeprom_data->total_liters[3]);
  zone.addSoilSensor(HUM_SNS_0, eeprom_data->cali_air[0], eeprom_data->cali_water[0]);
  zone.addSoilSensor(HUM_SNS_1, eeprom_data->cali_air[1], eeprom_data->cali_water[1]);
  zone.addSoilSensor(HUM_SNS_2, eeprom_data->cali_air[2], eeprom_data->cali_water[2]);
  zone.addSoilSensor(HUM_SNS_3, eeprom_data->cali_air[3], eeprom_data->cali_water[3]);
  
  zone.setOperationMode(static_cast<OperationModes>(eeprom_data->pump_mode[0]), PUMP_0);
  zone.setOperationMode(static_cast<OperationModes>(eeprom_data->pump_mode[1]), PUMP_1);
  zone.setOperationMode(static_cast<OperationModes>(eeprom_data->pump_mode[2]), PUMP_2);
  zone.setOperationMode(static_cast<OperationModes>(eeprom_data->pump_mode[3]), PUMP_3);

  zone.changeMoistureThreshold(eeprom_data->moisture_threshold[0], PUMP_0);
  zone.changeMoistureThreshold(eeprom_data->moisture_threshold[1], PUMP_1);
  zone.changeMoistureThreshold(eeprom_data->moisture_threshold[2], PUMP_2);
  zone.changeMoistureThreshold(eeprom_data->moisture_threshold[3], PUMP_3);
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

    // TODO maybe make telem also respond with succ or fail?
    // Standard commands, with response to user
    if (command == "TELEM") {
      // CMD: TELEM
      status = handleTelem(type);
    } else if (command == "SAV_EEP") {
      // CMD: SAV_EEP
      status = handleSaveEEPROM();
    } else if (command == "RST_EEP") {
      // CMD: RST_EEP
      status = handleResetEEPROM();
    } else if(command == "UPDT_FIRM") {
      // CMD: UPDT_FIRM
      status = handleUpdateFirmware();
    } else if (command == "MAN_IRR") {
      /*
        CMD: MAN_IRR, 
        PUMP: 32, 
        AMOUNT: 0.2
      */ 
      status = handleManualIrrigation(input);
    } else if (command == "CALI_SNSR") {
      /*
        CMD: CALI_SNSR, 
        SOIL_PIN: 32, 
        CALI_TYPE: air
      */ 
      status = handleCalibration(input);
    } else if(command == "CONFIG") {
      /*
        CMD: CONFIG,
        PUMP:
        SET_MODE: 3,
        CHG_DLY_LTR: 0.5,
        CHG_MOI_THR: 50,
      */
      status = handleConfig(input);
    } else if(command == "RSTRT") {
      // CMD: RSTRT
      ESP.restart();
    } else {
      DEBUG_PRINT("Unknown command: ");
      DEBUG_PRINTLN(command);
      status = false;
    }
    comms.write(response + (status ? "SUCC" : "FAIL"), type);
}

bool Interface::handleTelem(COMMS_TYPE type)
{
  comms.write(comms.getTelem(), type);
  comms.write(zone.getData(), type);
  DEBUG_PRINTLN("Sent telemetry data.");
  return true;
}

bool Interface::handleSaveEEPROM()
{
  EE_Data_t eeprom_data{};
  zone.saveToEEPROMData(&eeprom_data);

  Preferences prefs;
  prefs.begin("settings");

  if(prefs.isKey("EE_Data") && prefs.getBytesLength("EE_Data") == sizeof(EE_Data_t))
  {
    prefs.putBytes("EE_Data",
                    &eeprom_data,
                    sizeof(EE_Data_t));
  }
  else
  {
    DEBUG_PRINTLN("Using default settings");
  }

  prefs.end();
  zone.saveFromEEPROMData(&eeprom_data);
  return true;
}

bool Interface::handleResetEEPROM()
{
  // Memory load
  Preferences prefs;
  prefs.begin("settings");
  // EEPROM 
  EE_Data_t eeprom_data_local{};
  prefs.putBytes("EE_Data", &eeprom_data_local, sizeof(EE_Data_t));
  prefs.end();

  eeprom_data = &eeprom_data_local;
  zone.saveFromEEPROMData(eeprom_data);

  prefs.begin("wifi");
  prefs.putString("ssid", "");
  prefs.putString("pass", "");
  return true;
}

bool Interface::handleUpdateFirmware()
{
  comms.updateFirmware();
  return true;
}

bool Interface::handleManualIrrigation(String input)
{
  // TODO refactor
  DEBUG_PRINTLN("Started manual irrigating.");
  String pump_id = "";
  String amount = "";
  bool found_pump = Utils::findDataFromMessage(input, "PUMP:", pump_id);
  bool found_amount = Utils::findDataFromMessage(input, "AMOUNT:", amount);
  if(found_pump && found_amount)
  {
    zone.manualIrrigation(pump_id.toInt(), amount.toFloat());
    return true;
  } else {
    DEBUG_PRINTLN("Failed to MAN_IRR: find pump or amount");
    return false;
  }
}

bool Interface::handleConfig(String input)
{
  String pump_id = ""; 
  String mode = "";
  String chg_dly_ltr = "";
  String chg_moi_thr = "";
  // TODO proper status return
  bool found_pump = Utils::findDataFromMessage(input, "PUMP:", pump_id);
  bool found_mode = Utils::findDataFromMessage(input, "SET_MODE:", mode);
  bool found_chg_dly_ltr = Utils::findDataFromMessage(input, "CHG_DLY_LTR:", chg_dly_ltr);
  bool found_chg_moi_thr = Utils::findDataFromMessage(input, "CHG_MOI_THR:", chg_moi_thr);

  if(found_pump & found_mode & found_chg_dly_ltr & found_chg_moi_thr)
  {
    zone.setOperationMode(static_cast<OperationModes>(mode.toInt()), pump_id.toInt());
    zone.changeDailyLimit(pump_id.toInt(), chg_dly_ltr.toFloat());
    zone.changeMoistureThreshold(chg_moi_thr.toInt(), pump_id.toInt());
    DEBUG_PRINTLN("Changed Config");
    return true;
  } else {
    DEBUG_PRINTLN("ERROR: No pump found");
    return false;
  }
}

bool Interface::handleCalibration(String input) 
{
  String cali_type = "";
  String soil_pin = "";
  Utils::findDataFromMessage(input, "SOIL_PIN:", soil_pin);
  Utils::findDataFromMessage(input, "CALI_TYPE:", cali_type);

  if(cali_type == "AIR") {
    return zone.caliSoilInAir(soil_pin.toInt());
  } else if(cali_type == "WATER") {
    return zone.caliSoilInWater(soil_pin.toInt());
  } else {
    return false;
  }
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