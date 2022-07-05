#ifndef _AQUA_FAN_HARDWARE_H_
#define _AQUA_FAN_HARDWARE_H_

//----------------------------------------------------------------------------------------------
// display types
//----------------------------------------------------------------------------------------------

#define DISPLAY_SSD1306_128_64 0x01
#define DISPLAY_SSD1306_128_32 0x02

//----------------------------------------------------------------------------------------------
// water temperature sensor types
//----------------------------------------------------------------------------------------------

// Dallas Semiconductor / Maxim Integrated
#define WATER_TEMPERATURE_SENSOR_DS18B20    0x01   /* ±0.50°C */

// Melexis
#define WATER_TEMPERATURE_SENSOR_MLX90614   0x11   /* ±0.50°C / ±0.20°C */

//----------------------------------------------------------------------------------------------
// external sensor types (experimental)
//----------------------------------------------------------------------------------------------

// Microchip
#define EXTERNAL_SENSOR_MCP9808             0x21   /* ±0.25°C */

// Aosong Electronics Co.,Ltd / ASAIR
#define EXTERNAL_SENSOR_AHT10               0x31   /* ±0.30°C / ±2.0% Rh */
#define EXTERNAL_SENSOR_AHT15               0x32   /* ±0.30°C / ±2.0% Rh */
#define EXTERNAL_SENSOR_AHT20               0x33   /* ±0.30°C / ±2.0% Rh */

// Measurement Specialties / MEAS
#define EXTERNAL_SENSOR_HTU21               0x41   /* ±0.30°C / ±3.0% Rh */
#define EXTERNAL_SENSOR_HTU31               0x42   /* ±0.20°C / ±2.0% Rh */

// Texas Instruments
#define EXTERNAL_SENSOR_HDC1080             0x51   /* ±0.20°C / ±2.0% Rh */
#define EXTERNAL_SENSOR_HDC2080             0x52   /* ±0.20°C / ±2.0% Rh */

// Sensirion
#define EXTERNAL_SENSOR_SHT20               0x61   /* ±0.50°C / ±3.0% Rh */
#define EXTERNAL_SENSOR_SHT21               0x62   /* ±0.30°C / ±2.0% Rh */
#define EXTERNAL_SENSOR_SHT30               0x63   /* ±0.20°C / ±2.0% Rh */
#define EXTERNAL_SENSOR_SHT31               0x64   /* ±0.20°C / ±2.0% Rh */
#define EXTERNAL_SENSOR_SHT35               0x65   /* ±0.10°C / ±1.5% Rh */
#define EXTERNAL_SENSOR_SHT85               0x66   /* ±0.10°C / ±1.5% Rh */

// Bosch Sensortec
#define EXTERNAL_SENSOR_BMP180              0x71   /* ±0.50°C / ±1.0 hPA */
#define EXTERNAL_SENSOR_BMP280              0x72   /* ±0.50°C / ±1.0 hPA */
#define EXTERNAL_SENSOR_BME280              0x73   /* ±0.50°C / ±3.0% Rh / ±1.0 hPA */
#define EXTERNAL_SENSOR_BME680              0x74   /* ±0.50°C / ±3.0% Rh / ±0.6 hPA */

// GY-302
#define EXTERNAL_SENSOR_BH1750              0x81   /* ±1 Lx */

//----------------------------------------------------------------------------------------------
#endif   // _AQUA_FAN_HARDWARE_H_
