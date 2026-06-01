#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H
#pragma once

#include "Arduino.h"
#include "Utils.h"


class SoilSensor 
{
private:
    uint8_t pin;
    int moisture_raw = 0;
    long moisture_percent = 0;
    int cali_water_raw = -1;
    int cali_air_raw = -1;
public:

    SoilSensor(uint8_t pin, int cali_air, int cali_water);
    void checkRawValues();
    void calibrateInWater();
    void calibrateInAir();
    
    uint8_t getPin() {return pin;};
    long getMoisturePercent() {return moisture_percent;};
    int getMoistureRaw() {return moisture_raw;};
    int getCaliWater() {return cali_water_raw;};
    int getCaliAir() {return cali_air_raw;};
};

#endif
