#ifndef ZONE_H
#define ZONE_H
#pragma once

#include "Arduino.h"
#include "Pump.h"
#include "SoilSensor.h"
#include "WaterLevelSensor.h"

const int ARRAY_SIZE = 16;
const float MOISTURE_THRESHOLD = 0.1;

enum OperationModes {
    MODE_OFF = 0,
    MODE_MANUAL = 1, // Manual control from master, (which pump, how much)
    MODE_FLOOD = 2, // Floods tray of pot, until max daily limit reached (also use water level sensor for dry back)
    MODE_SOIL = 3 // Automatic based on soil moisture
};


class Zone 
{
private:
    int nr_pumps = 0;
    int nr_soil_sensors = 0;
    int nr_water_level_sensors = 0;
    float day_progressed = 0.0;
    float day_exact = 0.0;
    
    Pump *pumps[ARRAY_SIZE]; 
    SoilSensor *soil_sensors[ARRAY_SIZE];
    WaterLevelSensor *water_level_sensors[ARRAY_SIZE];

    OperationModes current_mode = MODE_OFF;
public:
    Zone();

    void addPump(int pin, float max_liter);
    void addSoilSensor(uint8_t pin);
    void addWaterLevelSensor(uint8_t pin);
    void startAutoIrrigating();
    void updateDay();
    
    String getData();
    String parseDataForWriting(String data_names[], String data_values[], int size);
    void resetDayProgression();
    void setOperationMode(OperationModes mode);
};

#endif
