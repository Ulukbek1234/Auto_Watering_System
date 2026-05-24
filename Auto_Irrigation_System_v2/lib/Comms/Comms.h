#ifndef COMMS_H
#define COMMS_H
#pragma once
#include <Config.h>
#include <Web.h>
#include <Bluetooth.h>
#include <SerialComms.h>


class Comms
{
private:
    HardwareSerial* serial;
    char input_buffer[256];
    COMMS_TYPE serial_type;

    Web *web;
    Bluetooth *bluetooth;
    SerialComms serial_comms;
public:
    Comms();
    void initComms();
    String read();
    void write(String output);
    HardwareSerial* getSource();
};

#endif