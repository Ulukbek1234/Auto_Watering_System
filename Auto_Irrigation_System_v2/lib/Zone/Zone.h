#ifndef ZONE_H
#define ZONE_H
#pragma once

#include "Arduino.h"
#include "Pump.h"
#include "SoilSensor.h"
#include "WaterLevelSensor.h"
#include "Utils.h"
#include "EEPROM.h"

const int ARRAY_SIZE = 16;
const float MOISTURE_THRESHOLD = 35.0;

enum OperationModes {
    MODE_OFF = 0,
    MODE_MANUAL = 1, // Manual control from master, (which pump, how much)
    MODE_FLOOD = 2, // Floods tray of pot, until max daily limit reached (also use water level sensor for dry back)
    MODE_SOIL = 3 // Automatic based on soil moisture
};

class Zone 
{
private:
    int zone_id = 0;
    int nr_pumps = 0;
    int nr_soil_sensors = 0;
    int nr_water_level_sensors = 0;
    float day_progressed = 0.0;
    float day_exact = 0.0;
    float total_day_progressed = 0.0;
    float eeprom_total_day_progressed = 0.0;
    float moisture_percent[ARRAY_SIZE] = {0.0f};
    float water_level_percent[ARRAY_SIZE] = {0.0f};
    
    Pump *pumps[ARRAY_SIZE]; 
    SoilSensor *soil_sensors[ARRAY_SIZE];
    WaterLevelSensor *water_level_sensors[ARRAY_SIZE];
    
    float eeprom_values[3];
    
    OperationModes current_mode = MODE_OFF;
    
public:
    int eeprom_offset_start = 0;
    int eeprom_offset_end = 0;

    Zone(int id, int absolute_offset);

    void addPump(int pin, float max_liter);
    void addSoilSensor(uint8_t pin);
    void addWaterLevelSensor(uint8_t pin);
    void startAutoIrrigating();
    void updateDay();
    
    String getData();
    String parseDataForWriting(String data_names[], String data_values[], int size);
    void setOperationMode(OperationModes mode);
    void updateSensors();
    void manualIrrigation(int pump_id, float amount);
    void saveToEEPROM();
    void changeDailyLimit(int pump_id, float new_limit);
    void resetEEPROM();
    void caliSoilInAir(int soil_pin);
    void caliSoilInWater(int soil_pin);

};

#endif
