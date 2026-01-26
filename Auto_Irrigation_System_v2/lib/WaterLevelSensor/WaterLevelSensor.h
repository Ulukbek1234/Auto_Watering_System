#ifndef WATER_LEVEL_SENSOR_H
#define WATER_LEVEL_SENSOR_H
#pragma once

#include "Arduino.h"
#include "Config.h"


class WaterLevelSensor 
{
private:
    uint8_t pin;
    int moister_raw;
    float moister_percent;

    public:
    WaterLevelSensor(uint8_t pin);
    void checkRawValues();
    
    uint8_t getPin() {return pin;};
    float getMoisterPercent() {return moister_percent;};
    float getMoisterRaw() {return moister_raw;};

};

#endif
