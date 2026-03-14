#ifndef UTILS_H
#define UTILS_H
#pragma once
#include <Arduino.h>
// #include <ArduinoJson.h>


typedef struct Dictionary 
{
    String key_;
    String value_;
} Dictionary;

struct EE_Data
{
  int cali_air[4] = {-1, -1, -1, -1};
  int cali_water[4] = {-1, -1, -1, -1};
  float total_liters[4] = {0.0, 0.0, 0.0, 0.0};
};

class Utils 
{
private:
public:
    static float floatMap(float x, float in_min, float in_max, float out_min, float out_max);
    static String parseDataForWriting(String data_names[], String data_values[], int size);
    static String parseDataForWriting(String data_name, String data_value);
    static String parseDataForWriting(Dictionary data);
    static String addCSVHeader(const char *header[], int size);
    static bool findDataFromMessage(const String& input, const String& key, String& output);
};

#endif