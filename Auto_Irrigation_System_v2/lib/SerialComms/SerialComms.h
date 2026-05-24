#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H
#pragma once
#include <Arduino.h>
#include <Config.h>


class SerialComms
{
private:
    HardwareSerial* serial;
    char input_buffer[256];
    COMMS_TYPE serial_type;
public:
    SerialComms();
    String read();
    void write(String output);
    HardwareSerial* getSource();
};

#endif