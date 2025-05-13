#ifndef ZONE_H
#define ZONE_H
#pragma once

#include "Arduino.h"
#include "Pump.h"
#include "SoilSensor.h"

const int ARRAY_SIZE = 1;

class Zone 
{
private:
    int nr_pumps = 0;
    int nr_soil_sensors = 0;
    float day = 0.0;
    float day_exact = 0.0;
    
    Pump *pumps[ARRAY_SIZE]; 
    SoilSensor *soil_sensors[ARRAY_SIZE];

public:
    Zone();

    void addPump(int pin, float max_liter);
    void addSoilSensor(int pin);
    void startAutoIrrigating();
    void updateDay();

    String getData();
    String parseDataForWriting(String data_names[], String data_values[], int size);
};

#endif
