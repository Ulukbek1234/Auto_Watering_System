#ifndef CONFIG_H
#define CONFIG_H
#pragma once

#define DEBUG 0

#define DEBUG_PRINT(x)    do { if (DEBUG) Serial.print(x); } while (0)
#define DEBUG_PRINTLN(x)  do { if (DEBUG) Serial.println(x); } while (0)
#define DEBUG_PRINTLNN(x, y) do { if (DEBUG) Serial.println(x, y); } while (0)

const int NR_DEC_POINTS = 8;
const int DRY_VALUE = 580;
const int WET_VALUE = 280;
const float epsilon = 0.000001;

#endif