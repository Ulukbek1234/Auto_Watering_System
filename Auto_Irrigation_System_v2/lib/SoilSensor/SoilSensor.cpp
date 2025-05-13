#include "SoilSensor.h"

SoilSensor::SoilSensor(int pin) 
{
    // Construct SoilSensor
    pinMode(INPUT, pin);
}

void SoilSensor::checkRawValues()
{
    moister_raw = analogRead(pin);
    moister_percent = map(moister_raw, DRY_VALUE, WET_VALUE, 0, 100);

    DEBUG_PRINT("SoilSensor moister_raw: ");
    DEBUG_PRINTLN(moister_raw);
    DEBUG_PRINT("SoilSensor moister_percent: ");
    DEBUG_PRINTLN(moister_percent);

}

