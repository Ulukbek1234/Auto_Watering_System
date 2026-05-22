#ifndef COMMS_H
#define COMMS_H
#pragma once
#include <Arduino.h>
#include "Web.h"
#include "Bluetooth.h"
#include "Config.h"

typedef enum {
    USB_SERIAL_COMMS,
    ESP32_SERIAL_COMMS,
    BLUETOOTH_COMMS,
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