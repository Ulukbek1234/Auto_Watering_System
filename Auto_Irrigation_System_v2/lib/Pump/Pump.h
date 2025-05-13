#ifndef PUMP_H
#define PUMP_H
#pragma once

#include "Arduino.h"
#include "Config.h"

#define PUMP_ON 0
#define PUMP_OFF 1

class Pump 
{
private:
    int pin;
    int status = 0; // 0 off, 1 on
    float liters_per_minute = 1.2;
    float max_liters = 3.0;  
    float daily_liter = 0.0;
public:
    Pump(int pin, float max_liters);
    void activatePump();
    void activatePump(float liters);

    void deactivatePump();
    void resetDailyLiter();


    int getPin()  {return pin;};
    float getDailyLiter() {return daily_liter;};
};

#endif
