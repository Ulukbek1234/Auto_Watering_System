#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H
#pragma once
#include <Arduino.h>
#include "Config.h"

typedef enum {
    USB_SERIAL_COMMS,
    ESP32_SERIAL_COMMS,
} CommsType;

class SerialComms
{
private:
    HardwareSerial* serial;
    char input_buffer[256];
    CommsType serial_type;
public:
    SerialComms(CommsType init_serial_type);
    String read();
    void write(String output);
    HardwareSerial* getSource();
};

#endif