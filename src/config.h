#ifndef _AQUA_FAN_CONFIG_H_
#define _AQUA_FAN_CONFIG_H_
//----------------------------------------------------------------------------------------------
#include "hardware.h"
//----------------------------------------------------------------------------------------------
// base hardware
//----------------------------------------------------------------------------------------------
#define DISPLAY_TYPE                           DISPLAY_SSD1306_128_64
#define PRIMARY_WATER_TEMPERATURE_SENSOR       WATER_TEMPERATURE_SENSOR_DS18B20
//#define SECONDARY_WATER_TEMPERATURE_SENSOR   WATER_TEMPERATURE_SENSOR_MLX90614
//----------------------------------------------------------------------------------------------
// external sensors (experimental)
//----------------------------------------------------------------------------------------------
//#define EXTERNAL_SENSOR_0                    EXTERNAL_SENSOR_BME280
//#define EXTERNAL_SENSOR_1                    EXTERNAL_SENSOR_SHT35
//#define EXTERNAL_SENSOR_2                    EXTERNAL_SENSOR_BH1750
//----------------------------------------------------------------------------------------------
// wifi support
//----------------------------------------------------------------------------------------------
//#define ENABLE_WIFI
//#define WIFI_SSID                            "guest"
//#define WIFI_PASSWORD                        "12345678"
//----------------------------------------------------------------------------------------------
#if __has_include("config.private.h")
  #include "config.private.h"
#endif
//----------------------------------------------------------------------------------------------
#endif   // _AQUA_FAN_CONFIG_H_
