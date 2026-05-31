#ifndef ZONE_H
#define ZONE_H
#pragma once

#include "Arduino.h"
#include <Pump.h>
#include <SoilSensor.h>
#include <WaterLevelSensor.h>
#include <Utils.h>
// #include "EEPROMEx.h"

const int ARRAY_SIZE = 4;


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
    float moisture_percent[ARRAY_SIZE] = {0.0f, 0.0f, 0.0f, 0.0f};
    float water_level_percent[ARRAY_SIZE] = {0.0f, 0.0f, 0.0f, 0.0f};
    float MOISTURE_THRESHOLD[ARRAY_SIZE] = {50.0, 50.0, 50.0, 50.0};
    
    Pump *pumps[ARRAY_SIZE]; 
    SoilSensor *soil_sensors[ARRAY_SIZE];
    WaterLevelSensor *water_level_sensors[ARRAY_SIZE];
    
    OperationModes current_mode[ARRAY_SIZE] = {MODE_MANUAL, MODE_MANUAL, MODE_MANUAL, MODE_MANUAL};

public:
    Zone(int id);
    
    void addEEPROMData(EE_Data_t eeprom_data);
    void addPump(int pin, float max_liter, float total_liters);
    void addSoilSensor(uint8_t pin, int cali_air, int cali_water);
    void addWaterLevelSensor(uint8_t pin);
    void startAutoIrrigation();
    void updateDay();
    
    String getData();
    String parseDataForWriting(String data_names[], String data_values[], int size);
    void setOperationMode(OperationModes mode, int pump_id);
    void updateSensors();
    void manualIrrigation(int pump_id, float amount);
    void saveToEEPROM(EE_Data_t *eeprom_data);
    void changeDailyLimit(int pump_id, float new_limit);
    void changeMoistureThreshold(float new_limit, int pump_id);
    void resetEEPROM();
    bool caliSoilInAir(int soil_pin);
    bool caliSoilInWater(int soil_pin);

};

#endif
