#include <Zone.h>
#include <Utils.h>
#include <Config.h>
#include <Interface.h>

// ESP32-specific includes
// #include <WiFi.h>
#include <Preferences.h>


// Web server;

unsigned long start_time = 0;
const unsigned long wait_time = 60000; // 1 minute


Interface *interface;

void setup() {
  // // comms->initComms();
  Serial.begin(115200);
  while (!Serial)
  {
    delay(100);
  }

  Serial.println("Work");
  // ESP32-specific setup
  // bluetooth = new Bluetooth();

  // server.begin();

  // Memory load
  Preferences prefs;
  prefs.begin("EE_Data");
  // EEPROM 
  EE_Data_t eeprom_data;
    
  // Read the data back
  size_t schLen = prefs.getBytesLength("EE_Data");
  char buffer[schLen];
  prefs.getBytes("EE_Data", buffer, schLen);
  
  // Cast the buffer back to the struct type
  EE_Data_t *retrieved = (EE_Data_t *)buffer;
  eeprom_data = *retrieved;
  
  
  interface = new Interface(eeprom_data);
  Serial.println("Running on ESP32");
  
  prefs.end();
  start_time = millis();
}


void loop() {
  // // server.loop();
  Serial.println("Loop");
  String command = interface->readCommand();



  // Non-Blocking delay logic
  if(millis() - start_time >= wait_time)
  {
    start_time = millis();
    DEBUG_PRINTLN("Waiting period over, checking sensors again.");
    interface->startAutoIrrigation();
    interface->updateDay();
  }
}
