#include "SoilSensor.h"

SoilSensor::SoilSensor(uint8_t pin) 
{
    // Construct SoilSensor
    pinMode(pin, INPUT);
    this->pin = pin;
}

void SoilSensor::checkRawValues()
{
    moisture_raw = analogRead(pin);
    moisture_percent = map(moisture_raw, DRY_VALUE_SOIL, WET_VALUE_SOIL, 0, 100);

    DEBUG_PRINT("SoilSensor moister_raw: ");
    DEBUG_PRINTLN(moisture_raw);
    DEBUG_PRINT("SoilSensor moister_percent: ");
    DEBUG_PRINTLN(moisture_percent);

}

