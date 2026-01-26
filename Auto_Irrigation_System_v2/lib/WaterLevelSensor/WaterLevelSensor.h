#ifndef WATER_LEVEL_SENSOR_H
#define WATER_LEVEL_SENSOR_H
#pragma once

#include "Arduino.h"
#include "Config.h"


class WaterLevelSensor 
{
private:
    uint8_t pin;
    int moisture_raw;
    float moisture_percent;

    public:
    WaterLevelSensor(uint8_t pin);
    void checkRawValues();
    
    uint8_t getPin() {return pin;};
    float getMoisturePercent() {return moisture_percent;};
    float getMoistureRaw() {return moisture_raw;};
};

#endif
