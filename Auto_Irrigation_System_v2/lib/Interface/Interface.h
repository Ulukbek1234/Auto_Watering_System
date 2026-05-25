#ifndef INTERFACE_H
#define INTERFACE_H
#pragma once

#include <Comms.h>
#include <Utils.h>
#include <Zone.h>
#include <Preferences.h>

class Interface 
{
private:
Comms comms;
Zone zone;
EE_Data_t *eeprom_data;

public:
    Interface(EE_Data_t eeprom_data_param);
    
    void commandHandler(String serial_input, COMMS_TYPE type);
    void startAutoIrrigation();
    void updateDay();
    void readCommand();
};

#endif