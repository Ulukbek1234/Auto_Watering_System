#ifndef COMMS_H
#define COMMS_H
#pragma once
#include <Arduino.h>
#include "Config.h"
#include "Web/Web.h"
#include "Bluetooth/Bluetooth.h"
#include "SerialComms/SerialComms.h"


class Comms
{
private:
    HardwareSerial* serial;
    char input_buffer[256];
    CommsType serial_type;

    Web *web;
    Bluetooth *bluetooth;
    SerialComms serial_comms;
public:
    Comms();
    String read();
    void write(String output);
    HardwareSerial* getSource();
};

#endif