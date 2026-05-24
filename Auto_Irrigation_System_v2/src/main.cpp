#include <Zone.h>
#include <Utils.h>
#include <Config.h>
#include "Interface.h"

// ESP32-specific includes
#include <WiFi.h>
#include <Preferences.h>


Preferences prefs;
Web server;

unsigned long start_time = 0;
const unsigned long wait_time = 60000; // 1 minute
// EEPROM 
EE_Data_t eeprom_data;

Interface *interface;

void setup() {
  // // comms->initComms();

  // ESP32-specific setup
  Serial.println("Running on ESP32");
  // bluetooth = new Bluetooth();

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

  interface = new Interface(eeprom_data);

  start_time = millis();
}


void loop() {
  // server.loop();
  
  // Check serial for commands from master here
  if(Serial.available() > 0) {
    String serial_input = Serial.readStringUntil('\n');
    DEBUG_PRINTLN("Received command: " + serial_input);
    serial_input.toUpperCase();
    serial_input.trim();
    interface->commandHandler(serial_input);
  }
  // Non-Blocking delay logic
  if(millis() - start_time >= wait_time)
  {
    start_time = millis();
    DEBUG_PRINTLN("Waiting period over, checking sensors again.");
    interface->startAutoIrrigation();
    interface->updateDay();
  }
}
