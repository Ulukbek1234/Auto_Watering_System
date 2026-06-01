#include <Utils.h>
#include <Preferences.h>
#include <Zone.h>
#include <Interface.h>

unsigned long start_time = 0;
const unsigned long wait_time = 60000; // 1 minute

Interface *interface;

void setup() {
  Serial.begin(115200);
  
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
  prefs.end();

  start_time = millis();
  DEBUG_PRINTLN("Running on ESP32");
}


void loop() {
  // // server.loop();
  interface->readCommand();
  delay(100);

  // Non-Blocking delay logic
  if(millis() - start_time >= wait_time)
  {
    start_time = millis();
    DEBUG_PRINTLN("Waiting period over, checking sensors again.");
    interface->startAutoIrrigation();
    interface->updateDay();
  }
}
