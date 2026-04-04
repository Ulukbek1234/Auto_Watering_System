#include "SoilSensor.h"

SoilSensor::SoilSensor(uint8_t pin, int cali_air, int cali_water) : cali_air_raw(cali_air), cali_water_raw(cali_water)
{
    // Construct SoilSensor
    pinMode(pin, INPUT);
    this->pin = pin;
}

void SoilSensor::checkRawValues()
{
    moisture_raw = analogRead(pin);
    // if (moisture_raw < 0 || moisture_raw > 1023) {
    //     DEBUG_PRINT("Invalid raw moisture value: ");
    //     DEBUG_PRINTLN(moisture_raw);
    //     moisture_percent = 0; // Set to 0% if invalid
    //     return;
    // }

    if(cali_air_raw == -1 || cali_water_raw == -1)
    {
        moisture_percent = map(moisture_raw, DRY_VALUE_SOIL, WET_VALUE_SOIL, 0, 100);
    }
    else
    {
        moisture_percent = map(moisture_raw, cali_air_raw, cali_water_raw, 0, 100);
    }

    DEBUG_PRINT("SoilSensor moister_raw: ");
    DEBUG_PRINTLN(moisture_raw);
    DEBUG_PRINT("SoilSensor moister_percent: ");
    DEBUG_PRINTLN(moisture_percent);
}

// TODO: 
/*
    1. Put sensor in water
    2. Put sensor in air
    3. Save values to EEPROM
*/
void SoilSensor::calibrateInWater()
{
    cali_water_raw = analogRead(pin);
}

void SoilSensor::calibrateInAir()
{
    cali_air_raw = analogRead(pin);
}