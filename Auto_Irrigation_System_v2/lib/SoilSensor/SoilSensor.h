#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H
#pragma once

#include "Arduino.h"
#include "Config.h"


class SoilSensor 
{
private:
    int pin;
    int moister_raw;
    float moister_percent;

    public:
    SoilSensor(int pin);
    void checkRawValues();
    
    int getPin() {return pin;};
    int getMoisterPercent() {return moister_percent;};

};

#endif
