#include <Arduino.h>

#include "Zone.h"
#include "Config.h"
#include "Utils.h"
#if defined(ESP32)
// ESP32-specific includes
#include <WiFi.h>
#include <Preferences.h>
#include "Web.h"
#include "Bluetooth.h"

#define PUMP_0 16
#define PUMP_1 17 
#define PUMP_2 18 
#define PUMP_3 19
#define HUM_SNS_0 32
#define HUM_SNS_1 33
#define HUM_SNS_2 34
#define HUM_SNS_3 35
Preferences prefs;
// Web server;
Bluetooth *bluetooth;
#elif defined(ARDUINO_ARCH_AVR)
// Arduino Uno/Nano includes
#include <Arduino.h>
#include "EEPROMex.h"
#define PUMP_0 8
#define PUMP_1 9 
#define PUMP_2 10 
#define PUMP_3 11
#define HUM_SNS_0 A0
#define HUM_SNS_1 A1
#define HUM_SNS_2 A2
#define HUM_SNS_3 A3

#endif


Zone *pots;
int nr_zones = 1; // TODO change dynamically
Zone *all_pots[1]; // TODO dynamic array if more pots needed in future
unsigned long start_time = 0;
const unsigned long wait_time = 60000; // 1 minute
// EEPROM 
EE_Data_t eeprom_data;


void setup() {

  #if defined(ESP32)
    // ESP32-specific setup
    Serial.begin(115200);
    Serial.println("Running on ESP32");
    bluetooth = new Bluetooth();

    // server.begin();

    // Memory load
    prefs.begin("EE_Data");
    
    // Read the data back
    size_t schLen = prefs.getBytesLength("EE_Data");
    char buffer[schLen];
    prefs.getBytes("EE_Data", buffer, schLen);
    
    // Cast the buffer back to the struct type
    EE_Data_t *retrieved = (EE_Data_t *)buffer;
    eeprom_data = *retrieved;
    
    prefs.end();


  #elif defined(ARDUINO_ARCH_AVR)
    Serial.begin(9600);

    Serial.println("Running on Arduino AVR");
    EEPROM.setMaxAllowedWrites(100);
    EEPROM.readBlock(0, eeprom_data);

    // Arduino-specific setup
  #endif





  pots = new Zone(0);
  pots->setOperationMode(MODE_MANUAL);

  pots->addPump(PUMP_0, 1.0);
  pots->addPump(PUMP_1, 1.0);
  pots->addPump(PUMP_2, 0.5);
  pots->addPump(PUMP_3, 0.0);
  
  pots->addSoilSensor(HUM_SNS_0, eeprom_data.cali_air[0], eeprom_data.cali_water[0]);
  pots->addSoilSensor(HUM_SNS_1, eeprom_data.cali_air[1], eeprom_data.cali_water[1]);
  pots->addSoilSensor(HUM_SNS_2, eeprom_data.cali_air[2], eeprom_data.cali_water[2]);
  pots->addSoilSensor(HUM_SNS_3, eeprom_data.cali_air[3], eeprom_data.cali_water[3]);
  
  all_pots[0] = pots;
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
      }

      #if defined(ESP32)
        // Store the struct as bytes
        prefs.putBytes("EE_Data", &eeprom_data, sizeof(eeprom_data));
      #elif defined(ARDUINO_ARCH_AVR)
        EEPROM.writeBlock(0, eeprom_data);

      #endif

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
        DEBUG_PRINTLN("Failed to NEW_MODE: Wrong operation mode");
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
        DEBUG_PRINTLN("Failed to MAN_IRR: find pump or amount");
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
        DEBUG_PRINTLN("Failed to CHG_DLY_LTR: find pump or limit");
        status = false;
      }
    } else if (command == "CHG_MOI_THR") {
      String new_limit = "";
      bool found_limit = Utils::findDataFromMessage(serial_input, "NEW_LIM:", new_limit);
      if(found_limit)
      {
        all_pots[zone]->changeMoistureThreshold(new_limit.toInt());
        status = true;
      } else {
        DEBUG_PRINTLN("Failed to CHG_MOI_THR: find limit");
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
  // server.loop();
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
