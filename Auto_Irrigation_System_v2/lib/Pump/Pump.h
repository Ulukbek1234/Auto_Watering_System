#ifndef PUMP_H
#define PUMP_H
#pragma once

#include "Arduino.h"
#include <Utils.h>

#define PUMP_ON 0
#define PUMP_OFF 1

const float LITERS_PER_MINUTE = 2.0;

class Pump 
{
private:
    int pin;
    int is_active = 0; // 0 off, 1 on
    float max_liters = 1.0; // Default, changed by init  
    float daily_liter = 0.0;
    float total_liter = 0.0; 
    bool limit_reached = false;

public:
    Pump(int pin, float max_liters_ee, float total_liters_ee);
    void turnOnPump(float liters, bool override);
    void deactivatePump();
    void activatePump();

    void turnOffPump();
    void resetDailyLiter();
    void updateMaxLiters(float new_max);

    int getPin()  {return pin;};
    float getDailyLiter() {return daily_liter;};
    float getTotalLiter() {return total_liter;};
    float getMaxLiter() {return max_liters;};
    int getIsActive() {return is_active;};
};

#endif
