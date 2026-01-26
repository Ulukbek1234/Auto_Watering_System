#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H
#pragma once

#include "Arduino.h"
#include "Config.h"

#define DRY_VALUE 300
#define WET_VALUE 700

class SoilSensor 
{
private:
    uint8_t pin;
    int moister_raw;
    long moister_percent;

    public:
    SoilSensor(uint8_t pin);
    void checkRawValues();
    
    uint8_t getPin() {return pin;};
    long getMoisterPercent() {return moister_percent;};

};

#endif
