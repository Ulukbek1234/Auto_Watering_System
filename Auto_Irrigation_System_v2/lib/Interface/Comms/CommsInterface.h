#ifndef COMMS_INTERFACE_H
#define COMMS_INTERFACE_H
#pragma once
#include "../Config/Config.h"
#include "Web/Web.h"
#include "Bluetooth/Bluetooth.h"
#include "SerialComms/SerialComms.h"


class CommsInterface
{
private:
    HardwareSerial* serial;
    char input_buffer[256];
    COMMS_TYPE serial_type;

    Web *web;
    Bluetooth *bluetooth;
    // SerialComms serial_comms;
public:
    CommsInterface();
    void initComms();
    String read();
    void write(String output);
    HardwareSerial* getSource();
};

#endif