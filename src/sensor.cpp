#include "sensor.h"
//----------------------------------------------------------------------------------------------
#ifndef AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
#include "controller.h"
//----------------------------------------------------------------------------------------------
#include <math.h>
//----------------------------------------------------------------------------------------------
// https://github.com/end2endzone/SoftTimers
#include <SoftTimers.h>
//----------------------------------------------------------------------------------------------
// minimum ambient temperature for errors control, °C
#define AMBIENT_TEMPERATURE_MIN_VALUE   1
// maximum ambient temperature for errors control, °C
#define AMBIENT_TEMPERATURE_MAX_VALUE   50
//----------------------------------------------------------------------------------------------
// minimum water temperature for errors control, °C
#define WATER_TEMPERATURE_MIN_VALUE   1
// maximum water temperature for errors control, °C
#define WATER_TEMPERATURE_MAX_VALUE   50
//----------------------------------------------------------------------------------------------
#if defined(WATER_TEMPERATURE_SENSOR) && !defined(PRIMARY_WATER_TEMPERATURE_SENSOR)
  #define PRIMARY_WATER_TEMPERATURE_SENSOR WATER_TEMPERATURE_SENSOR
#endif
//----------------------------------------------------------------------------------------------
#if defined(PRIMARY_WATER_TEMPERATURE_SENSOR) && defined(SECONDARY_WATER_TEMPERATURE_SENSOR)
  #if PRIMARY_WATER_TEMPERATURE_SENSOR == SECONDARY_WATER_TEMPERATURE_SENSOR
    #error "Primary and secondary water temperature sensors must be different"
  #endif
#endif
//----------------------------------------------------------------------------------------------

// DS18B20
#if PRIMARY_WATER_TEMPERATURE_SENSOR   == WATER_TEMPERATURE_SENSOR_DS18B20 || \
    SECONDARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_DS18B20

  #define HAVE_STANDARD_SENSOR
  #define HAVE_DS18B20

  // https://github.com/milesburton/Arduino-Temperature-Control-Library
  #include <DallasTemperature.h>

  #ifndef ONE_WIRE_PIN
    #ifdef AQUA_FAN_STANDALONE
      #define ONE_WIRE_PIN A3
    #else
      // for esp8266 do not use D0 and D8
      #define ONE_WIRE_PIN D3
    #endif
  #endif

  static OneWire oneWire(ONE_WIRE_PIN);
  static DallasTemperature ds18b20_sensors(&oneWire);

  #ifndef DS18B20_WATER_TEMPERATURE_SENSOR_INDEX
    #define DS18B20_WATER_TEMPERATURE_SENSOR_INDEX 0
  #endif

  #ifndef DS18B20_WATER_TEMPERATURE_SHIFT
    #define DS18B20_WATER_TEMPERATURE_SHIFT 0
  #endif

  #ifndef DS18B20_WATER_TEMPERATURE_FILTER
    // ~5 minutes per 99% of value per 3s interval
    #define DS18B20_WATER_TEMPERATURE_FILTER 22
  #endif

  static DeviceAddress ds18b20_water_address;
  static double        ds18b20_water_temperature          = WATER_TEMPERATURE_NULL_VALUE;
  static double        ds18b20_water_temperature_shift    = DS18B20_WATER_TEMPERATURE_SHIFT;
  static double        ds18b20_water_temperature_filtered = WATER_TEMPERATURE_NULL_VALUE;

#endif

// MLX90614
#if PRIMARY_WATER_TEMPERATURE_SENSOR   == WATER_TEMPERATURE_SENSOR_MLX90614 || \
    SECONDARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_MLX90614

  #define HAVE_FAST_SENSOR
  #define HAVE_MLX90614

  // GY-906 MLX90614-XXX, resolution 0.02°C
  // - BAA - precision ±0.50°C (single zone, 90° FOV)
  // - BCC - precision ±0.50°C (gradient compensated, 35° FOV)
  // - DCI - precision ±0.20°C (gradient compensated, 5° FOV)
  // https://github.com/adafruit/Adafruit-MLX90614-Library
  #include <Adafruit_MLX90614.h>

  static Adafruit_MLX90614 mlx90614_sensor;

  #ifndef MLX90614_WATER_TEMPERATURE_SHIFT
    #define MLX90614_WATER_TEMPERATURE_SHIFT 0
  #endif

  #ifndef MLX90614_WATER_TEMPERATURE_FILTER
    // ~5 minutes per 99% of value per 1s interval
    #define MLX90614_WATER_TEMPERATURE_FILTER 65
  #endif

  static double mlx90614_object_temperature           = WATER_TEMPERATURE_NULL_VALUE;
  static double mlx90614_object_temperature_shift     = MLX90614_WATER_TEMPERATURE_SHIFT;
  static double mlx90614_object_temperature_filtered  = WATER_TEMPERATURE_NULL_VALUE;

  #ifndef MLX90614_AMBIENT_TEMPERATURE_SHIFT
    #define MLX90614_AMBIENT_TEMPERATURE_SHIFT 0
  #endif

  static double mlx90614_ambient_temperature       = AMBIENT_TEMPERATURE_NULL_VALUE;
  static double mlx90614_ambient_temperature_shift = MLX90614_AMBIENT_TEMPERATURE_SHIFT;

#endif

//----------------------------------------------------------------------------------------------
// timers
//----------------------------------------------------------------------------------------------

#ifdef HAVE_FAST_SENSOR
  // timer for fast sensors
  static SoftTimer fast_sensor_timer;

  #ifndef FAST_SENSOR_UPDATE_INTERVAL
    // interval for fast sensor timer, ms
    #define FAST_SENSOR_UPDATE_INTERVAL 1000
  #endif
#endif

#ifdef HAVE_STANDARD_SENSOR
  // timer for standard sensors
  static SoftTimer standard_sensor_timer;

  #ifndef STANDARD_SENSOR_UPDATE_INTERVAL
    // interval for standard sensor timer, ms
    #define STANDARD_SENSOR_UPDATE_INTERVAL 3000
  #endif
#endif

//----------------------------------------------------------------------------------------------

bool sensorSetup() {

  bool result = true;

  #ifdef HAVE_MLX90614
    if (mlx90614_sensor.begin() == false) {
      result = false;
    } else {
      /**
       * water emissivity approx 0.96, but we set factory default
       * https://media.melexis.com/-/media/files/documents/application-notes/mlx90614-changing-emissivity-unlocking-key-application-note-melexis.pdf
       * https://www.melexis.com/-/media/files/documents/application-notes/thermal-mechanical-design-recommendations-ir-products-application-note-melexis.pdf
       *
       * E = (mlx90614_object_temperature ^ 4 - mlx90614_ambient_temperature ^ 4) / (ds18b20_external_water_temperature ^ 4 - mlx90614_ambient_temperature ^ 4)
      word ereg = mlx90614_sensor.readEmissivityReg();
      if (ereg != 65535) {
        mlx90614_sensor.writeEmissivityReg(65535);
      }
       */
    }
  #endif

  #ifdef HAVE_DS18B20
    ds18b20_sensors.begin();

    // the default resolution at power-up is 12-bit.
    // 9 / 10 / 11 / 12 bit => 0.5 / 0.25 / 0.125 / 0.0625°C
    if (ds18b20_sensors.getAddress(ds18b20_water_address, DS18B20_WATER_TEMPERATURE_SENSOR_INDEX) == false) {
      result = false;
    } else {
      ds18b20_sensors.setResolution(ds18b20_water_address, 12);
    }

  #endif

  #ifdef HAVE_FAST_SENSOR
    fast_sensor_timer.setTimeOutTime(FAST_SENSOR_UPDATE_INTERVAL);
    fast_sensor_timer.reset();
  #endif

  #ifdef HAVE_STANDARD_SENSOR
    standard_sensor_timer.setTimeOutTime(STANDARD_SENSOR_UPDATE_INTERVAL);
    standard_sensor_timer.reset();
  #endif

  return result;
}
//----------------------------------------------------------------------------------------------

double waterTemperatureFilter(double current, double previous, byte filter) {
  double result;
  if (current != WATER_TEMPERATURE_NULL_VALUE && previous != WATER_TEMPERATURE_NULL_VALUE) {
    result = (current + previous * (filter - 1)) / filter;
  } else {
    result = current;
  }

  return result;
}
//----------------------------------------------------------------------------------------------

#ifdef HAVE_MLX90614
double ambientTemperatureFilter(double current, double previous, byte filter) {
  double result;
  if (current != AMBIENT_TEMPERATURE_NULL_VALUE && previous != AMBIENT_TEMPERATURE_NULL_VALUE) {
    result = (current + previous * (filter - 1)) / filter;
  } else {
    result = current;
  }

  return result;
}
#endif   // HAVE_MLX90614
//----------------------------------------------------------------------------------------------

#ifdef HAVE_STANDARD_SENSOR
void updateStandardSensors() {
  #ifdef HAVE_DS18B20

    ds18b20_sensors.requestTemperaturesByAddress(ds18b20_water_address);

    ds18b20_water_temperature = ds18b20_sensors.getTempC(ds18b20_water_address);

    if (ds18b20_water_temperature == DEVICE_DISCONNECTED_C) {
      ds18b20_water_temperature = WATER_TEMPERATURE_NULL_VALUE;
    } else {
      ds18b20_water_temperature += ds18b20_water_temperature_shift;
      if (ds18b20_water_temperature < WATER_TEMPERATURE_MIN_VALUE || ds18b20_water_temperature > WATER_TEMPERATURE_MAX_VALUE) {
        ds18b20_water_temperature = WATER_TEMPERATURE_NULL_VALUE;
      }
    }

    ds18b20_water_temperature_filtered = waterTemperatureFilter(ds18b20_water_temperature, ds18b20_water_temperature_filtered, DS18B20_WATER_TEMPERATURE_FILTER);

  #endif   // HAVE_DS18B20
}
#endif   // HAVE_STANDARD_SENSOR
//----------------------------------------------------------------------------------------------

#ifdef HAVE_FAST_SENSOR
void updateFastSensors() {
  #ifdef HAVE_MLX90614

    static byte mlx90614_ambient_temperature_fails = 0;

    #ifndef MLX90614_AMBIENT_TEMPERATURE_MAX_FAILS
      #define MLX90614_AMBIENT_TEMPERATURE_MAX_FAILS (60000 / FAST_SENSOR_UPDATE_INTERVAL)
    #endif

    double ta = mlx90614_sensor.readAmbientTempC();
    if (isnan(ta) == 1) {
      mlx90614_ambient_temperature_fails++;
    } else {
      ta += mlx90614_ambient_temperature_shift;
      if (ta < AMBIENT_TEMPERATURE_MIN_VALUE || ta > AMBIENT_TEMPERATURE_MAX_VALUE) {
        mlx90614_ambient_temperature_fails++;
      } else {
        mlx90614_ambient_temperature       = ta;
        mlx90614_ambient_temperature_fails = 0;
      }
    }

    if (mlx90614_ambient_temperature_fails > MLX90614_AMBIENT_TEMPERATURE_MAX_FAILS) {
      mlx90614_ambient_temperature = AMBIENT_TEMPERATURE_NULL_VALUE;
    }

    static byte mlx90614_water_temperature_fails = 0;

    #ifndef MLX90614_WATER_TEMPERATURE_MAX_FAILS
      #define MLX90614_WATER_TEMPERATURE_MAX_FAILS MLX90614_AMBIENT_TEMPERATURE_MAX_FAILS
    #endif

    double tw = mlx90614_sensor.readObjectTempC();
    if (isnan(tw) == 1) {
      mlx90614_water_temperature_fails++;
    } else {
      tw += mlx90614_object_temperature_shift;
      if (tw < WATER_TEMPERATURE_MIN_VALUE || tw > WATER_TEMPERATURE_MAX_VALUE) {
        mlx90614_water_temperature_fails++;
      } else {
        mlx90614_object_temperature          = tw;
        mlx90614_water_temperature_fails     = 0;
        mlx90614_object_temperature_filtered = waterTemperatureFilter(mlx90614_object_temperature, mlx90614_object_temperature_filtered, MLX90614_WATER_TEMPERATURE_FILTER);
      }
    }

    if (mlx90614_water_temperature_fails > MLX90614_WATER_TEMPERATURE_MAX_FAILS) {
      mlx90614_object_temperature          = WATER_TEMPERATURE_NULL_VALUE;
      mlx90614_object_temperature_filtered = WATER_TEMPERATURE_NULL_VALUE;
    }

  #endif   // HAVE_MLX90614
}
#endif   // HAVE_FAST_SENSOR
//----------------------------------------------------------------------------------------------

void sensorLoop() {
  #ifdef HAVE_STANDARD_SENSOR
    if (standard_sensor_timer.hasTimedOut() == true) {
      updateStandardSensors();
      standard_sensor_timer.reset();
    }
  #endif

  #ifdef HAVE_FAST_SENSOR
    if (fast_sensor_timer.hasTimedOut() == true) {
      updateFastSensors();
      fast_sensor_timer.reset();
    }
  #endif
}
//----------------------------------------------------------------------------------------------

double primaryWaterTemperature() {
  #if PRIMARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_DS18B20
    return ds18b20_water_temperature_filtered;
  #elif PRIMARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_MLX90614
    return mlx90614_object_temperature_filtered;
  #else
    #error "Unsupported primary water temperature sensor"
  #endif
}
//----------------------------------------------------------------------------------------------

double secondaryWaterTemperature() {
  #ifdef SECONDARY_WATER_TEMPERATURE_SENSOR
    #if SECONDARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_MLX90614
      return mlx90614_object_temperature_filtered;
    #elif SECONDARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_DS18B20
      return ds18b20_water_temperature_filtered;
    #else
      #error "Unsupported secondary water temperature sensor"
    #endif
  #else
    return WATER_TEMPERATURE_NULL_VALUE;
  #endif
}

//----------------------------------------------------------------------------------------------
#ifdef AQUA_FAN_MASTER
//----------------------------------------------------------------------------------------------

void serializeSensors(JsonArray& json_sensors) {
  #ifdef HAVE_MLX90614
    if (mlx90614_object_temperature != WATER_TEMPERATURE_NULL_VALUE) {
      JsonObject json_sensor  = json_sensors.createNestedObject();
      json_sensor["sensor"]   = "MLX90614";
      json_sensor["type"]     = "water";
      json_sensor["value"]    = mlx90614_object_temperature;
      json_sensor["filtered"] = mlx90614_object_temperature_filtered;
      json_sensor["unit"]     = "C";
      json_sensor["role"]     =
      #if PRIMARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_MLX90614
        "primary";
      #else
        "secondary";
      #endif
    }

    if (mlx90614_ambient_temperature != AMBIENT_TEMPERATURE_NULL_VALUE) {
      JsonObject json_sensor = json_sensors.createNestedObject();
      json_sensor["sensor"]  = "MLX90614";
      json_sensor["type"]    = "ambient";
      json_sensor["value"]   = mlx90614_ambient_temperature;
      json_sensor["unit"]    = "C";
      json_sensor["role"]    = "primary";
    }
  #endif   // HAVE_MLX90614

  #ifdef HAVE_DS18B20
    if (ds18b20_water_temperature != WATER_TEMPERATURE_NULL_VALUE) {
      JsonObject json_sensor  = json_sensors.createNestedObject();
      json_sensor["sensor"]   = "DS18B20";
      json_sensor["type"]     = "water";
      json_sensor["value"]    = ds18b20_water_temperature;
      json_sensor["filtered"] = ds18b20_water_temperature_filtered;
      json_sensor["unit"]     = "C";
      json_sensor["role"]     =
      #if PRIMARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_DS18B20
        "primary";
      #else
        "secondary";
      #endif
    }
  #endif   // HAVE_DS18B20
}
//----------------------------------------------------------------------------------------------
#endif   // AQUA_FAN_MASTER
//----------------------------------------------------------------------------------------------
#endif   // !AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
