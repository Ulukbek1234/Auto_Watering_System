#ifndef UTILS_H
#define UTILS_H
#pragma once
#include <Arduino.h>
#include <map>

// ---------- Debug ----------
#ifndef DEBUG
  #define DEBUG 1
#endif

#if DEBUG
  #define DEBUG_PRINT(x)        do { Serial.print(x); } while (0)
  #define DEBUG_PRINTLN(x)      do { Serial.println(x); } while (0)
  // #define DEBUG_PRINTLNN(x, y)  do { Serial.println(x, y); } while (0)
#else
  #define DEBUG_PRINT(x)        do {} while (0)
  #define DEBUG_PRINTLN(x)      do {} while (0)
#endif

// ---------- Numeric ----------
constexpr float epsilon = 0.000001f;
constexpr int NR_DEC_POINTS = 8;

// ---------- Sensor calibration ----------
#define DRY_VALUE_SOIL  550
#define WET_VALUE_SOIL  300
#define DRY_VALUE_WATER 0
#define WET_VALUE_WATER 680


// Pins
#define PUMP_0 16
#define PUMP_1 17 
#define PUMP_2 18 
#define PUMP_3 19
#define HUM_SNS_0 32
#define HUM_SNS_1 33
#define HUM_SNS_2 34
#define HUM_SNS_3 35
//#define NR_ACTIVE_COMP 4


enum COMMS_TYPE {
    USB_SERIAL_COMMS,
    BLUETOOTH_COMMS,
    WEB_SOCKET_COMMS,
};

enum OperationModes {
    MODE_OFF = 0,
    MODE_MANUAL = 1, // Manual control from master, (which pump, how much)
    MODE_FLOOD = 2, // Floods tray of pot, until max daily limit reached (also use water level sensor for dry back)
    MODE_SOIL = 3 // Automatic based on soil moisture
};

typedef struct Dictionary 
{
    String key_;
    String value_;
} Dictionary;

typedef struct 
{
  int cali_air[4] = {-1, -1, -1, -1}; 
  int cali_water[4] = {-1, -1, -1, -1};
  float total_liters[4] = {0.0, 0.0, 0.0, 0.0};
  float max_liters[4] = {0.0, 0.0, 0.0, 0.0};
  float moisture_threshold[4] = {0.0, 0.0, 0.0, 0.0};
  int pump_mode[4] = {0, 0, 0, 0};
} EE_Data_t;

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