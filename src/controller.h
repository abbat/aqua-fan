#ifndef _AQUA_FAN_CONTROLLER_H_
#define _AQUA_FAN_CONTROLLER_H_
//----------------------------------------------------------------------------------------------
#include "aqua-fan.h"
//----------------------------------------------------------------------------------------------
#ifndef AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
bool controllerSetup();
void controllerLoop();
//----------------------------------------------------------------------------------------------
byte   cachedFanCount();
word   cachedFanRPM(byte i);
double cachedFanSpeed();
//----------------------------------------------------------------------------------------------
#ifdef AQUA_FAN_MASTER
  // https://arduinojson.org/v6/api/
  #include <ArduinoJson.h>

  void serializeFans(JsonObject& json_sensors);
#endif
//----------------------------------------------------------------------------------------------
#endif   // !AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
#endif   // _AQUA_FAN_CONTROLLER_H_
