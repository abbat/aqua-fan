#ifndef _AQUA_FAN_H_
#define _AQUA_FAN_H_
//----------------------------------------------------------------------------------------------
#include "config.h"
//----------------------------------------------------------------------------------------------
#include <Arduino.h>
//----------------------------------------------------------------------------------------------
#ifdef ENABLE_WIFI
  // if you use esp8266 for wifi, fans must be controlled by separated arduino board via i2c
  // and your esp8266 is always i2c master and separated arduino board is always i2c slave
  #ifdef ARDUINO_ARCH_AVR
    #define AQUA_FAN_SLAVE
  #else
    #define AQUA_FAN_MASTER
  #endif
#else
  #define AQUA_FAN_STANDALONE
#endif
//----------------------------------------------------------------------------------------------
#endif   // _AQUA_FAN_H_
