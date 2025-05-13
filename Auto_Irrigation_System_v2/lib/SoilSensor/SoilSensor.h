#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H
#pragma once

#include "Arduino.h"
#include "Config.h"


class SoilSensor 
{
private:
    uint8_t pin;
    int moister_raw;
    float moister_percent;

    public:
    SoilSensor(uint8_t pin);
    void checkRawValues();
    
    uint8_t getPin() {return pin;};
    float getMoisterPercent() {return moister_percent;};

};

#endif
