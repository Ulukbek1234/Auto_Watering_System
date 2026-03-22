#ifndef ZONE_H
#define ZONE_H
#pragma once

#include "Arduino.h"
#include "Pump.h"
#include "SoilSensor.h"
#include "WaterLevelSensor.h"
#include "Utils.h"
// #include "EEPROMEx.h"

const int ARRAY_SIZE = 16;

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
    float MOISTURE_THRESHOLD = 40.0;
    
    Pump *pumps[ARRAY_SIZE]; 
    SoilSensor *soil_sensors[ARRAY_SIZE];
    WaterLevelSensor *water_level_sensors[ARRAY_SIZE];
    
    OperationModes current_mode = MODE_OFF;

public:
    Zone(int id);

    void addPump(int pin, float max_liter, float ee_total_liters = 0.0);
    void addSoilSensor(uint8_t pin, int cali_air = -1, int cali_water = -1);
    void addWaterLevelSensor(uint8_t pin);
    void startAutoIrrigating();
    void updateDay();
    
    String getData();
    String parseDataForWriting(String data_names[], String data_values[], int size);
    void setOperationMode(OperationModes mode);
    void updateSensors();
    void manualIrrigation(int pump_id, float amount);
    void saveToEEPROM(EE_Data *eeprom_data);
    void changeDailyLimit(int pump_id, float new_limit);
    void changeMoistureThreshold(float new_limit);
    void resetEEPROM();
    bool caliSoilInAir(int soil_pin);
    bool caliSoilInWater(int soil_pin);

};

#endif
