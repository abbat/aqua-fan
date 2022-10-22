#ifndef _AQUA_FAN_SENSOR_H_
#define _AQUA_FAN_SENSOR_H_
//----------------------------------------------------------------------------------------------
#include "aqua-fan.h"
//----------------------------------------------------------------------------------------------
#ifndef AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
// unknown / null ambient temperature, °C
#define AMBIENT_TEMPERATURE_NULL_VALUE -274
//----------------------------------------------------------------------------------------------
// unknown / null water temperature, °C
#define WATER_TEMPERATURE_NULL_VALUE -274
//----------------------------------------------------------------------------------------------
// unknown / null humidity, %
#define HUMIDITY_NULL_VALUE -1
//----------------------------------------------------------------------------------------------
// unknown / null pressure, hPa
#define PRESSURE_NULL_VALUE -1
// Pa in mm Hg
#define PRESSURE_PA_IN_MMHG 133.3223684
//----------------------------------------------------------------------------------------------
bool sensorSetup();
void sensorLoop();
//----------------------------------------------------------------------------------------------
double primaryWaterTemperature();
//----------------------------------------------------------------------------------------------
bool   haveSecondaryWaterTemperature();
double secondaryWaterTemperature();
//----------------------------------------------------------------------------------------------
bool   haveAmbientTemperature();
double ambientTemperature();
//----------------------------------------------------------------------------------------------
bool   haveHumidity();
double humidity();
//----------------------------------------------------------------------------------------------
bool   havePressure();
double pressure();
//----------------------------------------------------------------------------------------------
#ifdef AQUA_FAN_MASTER
  // https://arduinojson.org/v6/api/
  #include <ArduinoJson.h>

  void serializeSensors(JsonArray& json_sensors);
#endif   // AQUA_FAN_MASTER
//----------------------------------------------------------------------------------------------
#endif   // !AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
#endif   // _AQUA_FAN_SENSOR_H_
