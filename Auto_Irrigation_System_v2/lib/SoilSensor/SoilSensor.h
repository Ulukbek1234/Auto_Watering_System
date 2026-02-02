#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H
#pragma once

#include "Arduino.h"
#include "Config.h"


class SoilSensor 
{
private:
    uint8_t pin;
    int moisture_raw;
    long moisture_percent;

    public:
    SoilSensor(uint8_t pin);
    void checkRawValues();
    
    uint8_t getPin() {return pin;};
    long getMoisturePercent() {return moisture_percent;};
    int getMoistureRaw() {return moisture_raw;};
};

#endif
