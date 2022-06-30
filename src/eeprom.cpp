#include "eeprom.h"
//----------------------------------------------------------------------------------------------
#ifndef AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
#include <EEPROM.h>
//----------------------------------------------------------------------------------------------
static word eeprom_water_temperature_base;   // eeprom value for water temperature base in 0.1°C units (245 = 24.5°C)
static byte eeprom_min_fan_speed;            // eeprom value for min fan speed in percents (0 - 100)
static byte eeprom_max_fan_speed;            // eeprom value for max fan speed in percents (0 - 100)
//----------------------------------------------------------------------------------------------
#define EEPROM_BEGIN                         0
#define EEPROM_WATER_TEMPERATURE_BASE_OFFSET (EEPROM_BEGIN)
#define EEPROM_MIN_FAN_SPEED_OFFSET          (EEPROM_WATER_TEMPERATURE_BASE_OFFSET + sizeof(eeprom_water_temperature_base))
#define EEPROM_MAX_FAN_SPEED_OFFSET          (EEPROM_MIN_FAN_SPEED_OFFSET + sizeof(eeprom_min_fan_speed))
#define EEPROM_END                           (EEPROM_MAX_FAN_SPEED_OFFSET + sizeof(eeprom_max_fan_speed))
#define EEPROM_SIZE                          (EEPROM_END - EEPROM_BEGIN)
//----------------------------------------------------------------------------------------------

#ifndef DEFAULT_WATER_TEMPERATURE_BASE
  // 25°C is default recommended aquarium water temperature
  #define DEFAULT_WATER_TEMPERATURE_BASE 25
#endif

// water temperature base in 0.1°C units (245 = 24.5°C)
static word water_temperature_base = DEFAULT_WATER_TEMPERATURE_BASE * 10;
// water temperature base
static double water_temperature_base_f = DEFAULT_WATER_TEMPERATURE_BASE;

// default min fan speed, %
#ifndef DEFAULT_MIN_FAN_SPEED
  #define DEFAULT_MIN_FAN_SPEED 0
#endif

// min fan speed, %
static byte min_fan_speed = DEFAULT_MIN_FAN_SPEED;
// min fan speed, 1/%
static double min_fan_speed_f = (double)min_fan_speed / 100;

// default max fan speed, %
#ifndef DEFAULT_MAX_FAN_SPEED
  #define DEFAULT_MAX_FAN_SPEED 80
#endif

// max fan speed, %
static byte max_fan_speed = DEFAULT_MAX_FAN_SPEED;
// max fan speed, 1/%
static double max_fan_speed_f = (double)max_fan_speed / 100;

//----------------------------------------------------------------------------------------------

byte eepromReadByte(size_t offset) {
  return EEPROM.read(offset);
}
//----------------------------------------------------------------------------------------------

void eepromWriteByte(size_t offset, byte value) {
  EEPROM.write(offset, value);
}
//----------------------------------------------------------------------------------------------

word eepromReadWord(size_t offset) {
  return word(
    eepromReadByte(offset),
    eepromReadByte(offset + sizeof(byte))
  );
}
//----------------------------------------------------------------------------------------------

void eepromWriteWord(size_t offset, word value) {
  eepromWriteByte(offset, highByte(value));
  eepromWriteByte(offset + sizeof(byte), lowByte(value));
}
//----------------------------------------------------------------------------------------------

bool eepromSetup() {
  #ifdef AQUA_FAN_MASTER
    // esp8266 specific
    EEPROM.begin(EEPROM_SIZE);
  #endif

  eeprom_water_temperature_base = eepromReadWord(EEPROM_WATER_TEMPERATURE_BASE_OFFSET);
  setWaterTemperatureBase10(eeprom_water_temperature_base);

  eeprom_min_fan_speed = eepromReadByte(EEPROM_MIN_FAN_SPEED_OFFSET);
  setMinFanSpeed(eeprom_min_fan_speed);

  eeprom_max_fan_speed = eepromReadByte(EEPROM_MAX_FAN_SPEED_OFFSET);
  setMaxFanSpeed(eeprom_max_fan_speed);

  return true;
}
//----------------------------------------------------------------------------------------------

double waterTemperatureBase() {
  return water_temperature_base_f;
}
//----------------------------------------------------------------------------------------------

word waterTemperatureBase10() {
  return water_temperature_base;
}
//----------------------------------------------------------------------------------------------

bool setWaterTemperatureBase10(word base) {
  if (base < MIN_WATER_TEMPERATURE_BASE_10 || base > MAX_WATER_TEMPERATURE_BASE_10) {
    return false;
  }

  water_temperature_base   = base;
  water_temperature_base_f = (double)water_temperature_base / 10.0;

  return true;
}
//----------------------------------------------------------------------------------------------

bool saveWaterTemperatureBase() {
  if (eeprom_water_temperature_base != water_temperature_base) {
    eeprom_water_temperature_base = water_temperature_base;
    eepromWriteWord(EEPROM_WATER_TEMPERATURE_BASE_OFFSET, eeprom_water_temperature_base);
    return true;
  }

  return false;
}
//----------------------------------------------------------------------------------------------

byte minFanSpeed() {
  return min_fan_speed;
}
//----------------------------------------------------------------------------------------------

double minFanSpeedF() {
  return min_fan_speed_f;
}
//----------------------------------------------------------------------------------------------

bool setMinFanSpeed(byte speed) {
  if (speed > 100 || speed > max_fan_speed) {
    return false;
  }

  min_fan_speed   = speed;
  min_fan_speed_f = (double)min_fan_speed / 100.0;

  return true;
}
//----------------------------------------------------------------------------------------------

bool saveMinFanSpeed() {
  if (eeprom_min_fan_speed != min_fan_speed) {
    eeprom_min_fan_speed = min_fan_speed;
    eepromWriteByte(EEPROM_MIN_FAN_SPEED_OFFSET, eeprom_min_fan_speed);
    return true;
  }

  return false;
}
//----------------------------------------------------------------------------------------------

byte maxFanSpeed() {
  return max_fan_speed;
}
//----------------------------------------------------------------------------------------------

double maxFanSpeedF() {
  return max_fan_speed_f;
}
//----------------------------------------------------------------------------------------------

bool setMaxFanSpeed(byte speed) {
  if (speed > 100 || speed < min_fan_speed) {
    return false;
  }

  max_fan_speed   = speed;
  max_fan_speed_f = (double)max_fan_speed / 100.0;

  return true;
}
//----------------------------------------------------------------------------------------------

bool saveMaxFanSpeed() {
  if (eeprom_max_fan_speed != max_fan_speed) {
    eeprom_max_fan_speed = max_fan_speed;
    eepromWriteByte(EEPROM_MAX_FAN_SPEED_OFFSET, eeprom_max_fan_speed);
    return true;
  }

  return false;
}
//----------------------------------------------------------------------------------------------

void eepromCommit() {
  saveWaterTemperatureBase();
  saveMinFanSpeed();
  saveMaxFanSpeed();

  #ifdef AQUA_FAN_MASTER
    // esp8266 specific
    EEPROM.commit();
  #endif
}
//----------------------------------------------------------------------------------------------
#endif   // !AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
