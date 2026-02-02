#ifndef CONFIG_H
#define CONFIG_H
#pragma once

// ---------- Debug ----------
#ifndef DEBUG
  #define DEBUG 1
#endif

#if DEBUG
  #define DEBUG_PRINT(x)        do { Serial.print(x); } while (0)
  #define DEBUG_PRINTLN(x)      do { Serial.println(x); } while (0)
  #define DEBUG_PRINTLNN(x, y)  do { Serial.println(x, y); } while (0)
#else
  #define DEBUG_PRINT(x)        do {} while (0)
  #define DEBUG_PRINTLN(x)      do {} while (0)
  #define DEBUG_PRINTLNN(x, y)  do {} while (0)
#endif

// ---------- Numeric ----------
#define NR_DEC_POINTS   8
#define epsilon         0.000001f   

// ---------- Sensor calibration ----------
#define DRY_VALUE_SOIL  550
#define WET_VALUE_SOIL  300
#define DRY_VALUE_WATER 0
#define WET_VALUE_WATER 680

#endif
