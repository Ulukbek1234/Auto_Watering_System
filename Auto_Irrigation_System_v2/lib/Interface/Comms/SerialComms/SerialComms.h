#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H
#pragma once
#include <Arduino.h>
// #include "../../Config/Config.h"


class SerialComms
{
private:
    //HardwareSerial* serial;
    char input_buffer[256];
    // COMMS_TYPE serial_type;
public:
    SerialComms();
    // void initComms(COMMS_TYPE init_serial_type);
    // String read();
    // void write(String output);
    // HardwareSerial* getSource();
};

#endif