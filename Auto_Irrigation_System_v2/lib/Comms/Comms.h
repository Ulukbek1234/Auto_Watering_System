#ifndef COMMS_H
#define COMMS_H
#pragma once
#include <Utils.h>
#include <Web.h>
#include <Bluetooth.h>
#include <SerialComms.h>


class Comms
{
private:
    HardwareSerial* serial;
    char input_buffer[256];
    
    Web web;
    Bluetooth *bluetooth;
    SerialComms serial_comms;
public:
    static const int nr_active_types = 2; // TODO dynamically adjust 
    COMMS_TYPE active_types[nr_active_types];
    Comms();
    String read(COMMS_TYPE comms_type);
    void write(String output, COMMS_TYPE type);
    HardwareSerial* getSource();
    void updateFirmware();
    String getTelem();

};

#endif