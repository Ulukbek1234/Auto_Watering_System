#include "WaterLevelSensor.h"

WaterLevelSensor::WaterLevelSensor(uint8_t pin) 
{
    // Construct WaterLevelSensor
}

void WaterLevelSensor::checkRawValues()
{
    moister_raw = analogRead(pin);
    moister_percent = map(moister_raw, DRY_VALUE, WET_VALUE, 0, 100);

    DEBUG_PRINT("WaterLevelSensor moister_raw: ");
    DEBUG_PRINTLN(moister_raw);
    DEBUG_PRINT("WaterLevelSensor moister_percent: ");
    DEBUG_PRINTLN(moister_percent);
}

