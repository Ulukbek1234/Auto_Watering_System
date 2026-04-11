#include "Pump.h"


Pump::Pump(int pin, float max_liters, float ee_total_liters) 
{
    // Construct Pump
    this->pin = pin;
    this->max_liters = max_liters;
    this->limit_reached = false;
    this->total_liter = ee_total_liters;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, PUMP_OFF);
    activatePump();
}


void Pump::turnOnPump(float liters, bool override = false)
{
    DEBUG_PRINTLN("Started pumping liters");
    if (!override &&
    (
        is_active == 0 ||
        limit_reached ||
        (daily_liter - max_liters) > epsilon
    ))
    {
        DEBUG_PRINTLN("Max daily limit reached or not active");
        limit_reached = true;
        return;
    }
    total_liter += liters;
    // EEPROM.write(pin, total_liter);
    daily_liter += liters;
    
    float seconds_needed = (60.0 / LITERS_PER_MINUTE) * liters;
    unsigned long duration_ms = (unsigned long)(seconds_needed * 1000);
    
    unsigned long start_time = millis();
    unsigned long end_time = start_time + duration_ms;
    
    digitalWrite(this->pin, PUMP_ON);

    while (millis() < end_time) {
        delay(10); // avoid CPU hogging
    }
    turnOffPump();
}

void Pump::turnOffPump()
{
    DEBUG_PRINTLN("Stopped pumping");
    digitalWrite(this->pin, PUMP_OFF);
}

void Pump::resetDailyLiter()
{
    daily_liter = 0.0;
    limit_reached = false;
}

void Pump::activatePump()
{
    DEBUG_PRINTLN("Pump activated");
    is_active = true;
}

void Pump::deactivatePump()
{
    DEBUG_PRINTLN("Pump deactivated");
    is_active = false;
}

void Pump::updateMaxLiters(float new_max)
{
    limit_reached = false; 
    max_liters = new_max;
}