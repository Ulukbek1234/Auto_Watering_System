#include "Comms.h"

Comms::Comms() {
    #ifdef ESP32
    serial_comms = new SerialComms(ESP32_SERIAL_COMMS);
    // web = new Web(80);
    // bluetooth = new Bluetooth();
    
    #elif defined(ARDUINO_ARCH_AVR)
    serial_comms = new SerialComms(USB_SERIAL_COMMS);    
    #endif
}