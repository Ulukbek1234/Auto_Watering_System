#ifndef CONFIG_H
#define CONFIG_H
#pragma once

#define DEBUG 0

#define DEBUG_PRINT(x)    do { if (DEBUG) Serial.print(x); } while (0)
#define DEBUG_PRINTLN(x)  do { if (DEBUG) Serial.println(x); } while (0)
#define DEBUG_PRINTLNN(x, y) do { if (DEBUG) Serial.println(x, y); } while (0)

const int NR_DEC_POINTS = 8;
const float epsilon = 0.000001;

#define DRY_VALUE_SOIL 300
#define WET_VALUE_SOIL 700
#define DRY_VALUE_WATER 0
#define WET_VALUE_WATER 1200

#endif