#ifndef PUMP_H
#define PUMP_H
#pragma once

#include "Arduino.h"
#include "Config.h"

#define PUMP_ON 0
#define PUMP_OFF 1

const float LITERS_PER_MINUTE = 2.0;

class Pump 
{
private:
    int pin;
    int status = 0; // 0 off, 1 on
    float max_liters = 1.0; // Default, changed by init  
    float daily_liter = 0.0;
    float total_liter = 0.0; // TODO safe to EEPROM
    bool limit_reached = false;
public:
    Pump(int pin, float max_liters);
    void activatePump();
    void activatePump(float liters);

    void deactivatePump();
    void resetDailyLiter();


    int getPin()  {return pin;};
    float getDailyLiter() {return daily_liter;};
    float getTotalLiter() {return total_liter;};
    float getMaxLiter() {return max_liters;};
};

#endif
