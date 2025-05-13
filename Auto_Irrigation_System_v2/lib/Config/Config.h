#ifndef CONFIG_H
#define CONFIG_H
#pragma once

#define DEBUG 1

#define DEBUG_PRINT(x)    do { if (DEBUG) Serial.print(x); } while (0)
#define DEBUG_PRINTLN(x)  do { if (DEBUG) Serial.println(x); } while (0)

const int NR_DEC_POINTS = 8;
const int DRY_VALUE = 580;
const int WET_VALUE = 280;


#endif