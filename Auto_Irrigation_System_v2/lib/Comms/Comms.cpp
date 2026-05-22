#include "Comms.h"

Comms::Comms() {
    #ifdef ESP32
    web = new Web(80);
    bluetooth = new Bluetooth();
    serialComms = new SerialComms(ESP32_SERIAL_COMMS);
    
    #endif
    elif defined(ARDUINO_ARCH_AVR)
    serialComms = new SerialComms(USB_SERIAL_COMMS);    
    #endif
}