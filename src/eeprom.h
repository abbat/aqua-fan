#ifndef _AQUA_FAN_EEPROM_H_
#define _AQUA_FAN_EEPROM_H_
//----------------------------------------------------------------------------------------------
#include "aqua-fan.h"
//----------------------------------------------------------------------------------------------
#ifndef AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
#define MIN_WATER_TEMPERATURE_BASE    20
#define MIN_WATER_TEMPERATURE_BASE_10 (MIN_WATER_TEMPERATURE_BASE * 10)
#define MAX_WATER_TEMPERATURE_BASE    30
#define MAX_WATER_TEMPERATURE_BASE_10 (MAX_WATER_TEMPERATURE_BASE * 10)
//----------------------------------------------------------------------------------------------
bool eepromSetup();
void eepromCommit();
//----------------------------------------------------------------------------------------------
double waterTemperatureBase();                 // return base temperature °C
word   waterTemperatureBase10();               // return base temperature in 0.1°C units (245 = 24.5°C)
bool   setWaterTemperatureBase10(word base);   // set base temperature in 0.1°C units (245 = 24.5°C)
//----------------------------------------------------------------------------------------------
byte   minFanSpeed();                          // return min fan speed in percent
double minFanSpeedF();                         // return relative min fan speed (0..1)
bool   setMinFanSpeed(byte speed);             // set min fan speed in percent
//----------------------------------------------------------------------------------------------
byte   maxFanSpeed();                          // return max fan speed in percent
double maxFanSpeedF();                         // return relative max fan speed (0..1)
bool   setMaxFanSpeed(byte speed);             // set max fan speed in percent
//----------------------------------------------------------------------------------------------
#endif   // !AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
#endif   // _AQUA_FAN_EEPROM_H_
