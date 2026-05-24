#include "WaterLevelSensor.h"

WaterLevelSensor::WaterLevelSensor(uint8_t pin) 
{
    // Construct WaterLevelSensor
    this->pin = pin;
}

void WaterLevelSensor::checkRawValues()
{
    moisture_raw = analogRead(pin);
    moisture_percent = map(moisture_raw, DRY_VALUE_WATER, WET_VALUE_WATER, 0, 100);

    DEBUG_PRINT("WaterLevelSensor moister_raw: ");
    DEBUG_PRINTLN(moisture_raw);
    DEBUG_PRINT("WaterLevelSensor moister_percent: ");
    DEBUG_PRINTLN(moisture_percent);
}

