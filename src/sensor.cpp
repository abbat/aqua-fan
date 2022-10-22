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
// minimum humidity for errors control, %
#define HUMIDITY_MIN_VALUE  0
// maximum humidity for errors control, %
#define HUMIDITY_MAX_VALUE  100
//----------------------------------------------------------------------------------------------
// minimum pressure for errors control, Pa (~637.5 mm Hg)
#define PRESSURE_MIN_VALUE  85000
// maximum pressure for errors control, Pa (~817.5 mm Hg)
#define PRESSURE_MAX_VALUE  109000
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

  #define HAVE_DS18B20
  #define HAVE_STANDARD_SENSOR

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

#endif   // DS18B20

// MLX90614
#if PRIMARY_WATER_TEMPERATURE_SENSOR   == WATER_TEMPERATURE_SENSOR_MLX90614 || \
    SECONDARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_MLX90614

  #define HAVE_MLX90614
  #define HAVE_FAST_SENSOR

  #ifndef HAVE_AMBIENT_SENSOR
    #define HAVE_AMBIENT_SENSOR
  #endif

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

#endif   // MLX90614

#ifdef EXTERNAL_SENSOR_0
  #ifdef EXTERNAL_SENSOR_1
    #if EXTERNAL_SENSOR_0 == EXTERNAL_SENSOR_1
      #error "EXTERNAL_SENSOR_0 and EXTERNAL_SENSOR_1 can not be same"
    #endif
  #endif
  #ifdef EXTERNAL_SENSOR_2
    #if EXTERNAL_SENSOR_0 == EXTERNAL_SENSOR_2
      #error "EXTERNAL_SENSOR_0 and EXTERNAL_SENSOR_2 can not be same"
    #endif
  #endif
#endif

#ifdef EXTERNAL_SENSOR_1
  #ifdef EXTERNAL_SENSOR_2
    #if EXTERNAL_SENSOR_1 == EXTERNAL_SENSOR_2
      #error "EXTERNAL_SENSOR_1 and EXTERNAL_SENSOR_2 can not be same"
    #endif
  #endif
#endif

// BME280
#if EXTERNAL_SENSOR_0 == EXTERNAL_SENSOR_BME280 || \
    EXTERNAL_SENSOR_1 == EXTERNAL_SENSOR_BME280 || \
    EXTERNAL_SENSOR_2 == EXTERNAL_SENSOR_BME280

  #define HAVE_BME280
  #define HAVE_SLOW_SENSOR

  #ifndef HAVE_AMBIENT_SENSOR
    #define HAVE_AMBIENT_SENSOR
  #endif

  #ifndef HAVE_HUMIDITY_SENSOR
    #define HAVE_HUMIDITY_SENSOR
  #endif

  #ifndef HAVE_PRESSURE_SENSOR
    #define HAVE_PRESSURE_SENSOR
  #endif

  // https://github.com/adafruit/Adafruit_BME280_Library
  #include <Adafruit_BME280.h>

  static Adafruit_BME280 bme280_sensor;

  #ifndef BME280_AMBIENT_TEMPERATURE_SHIFT
    #define BME280_AMBIENT_TEMPERATURE_SHIFT 0
  #endif

  static double bme280_temperature       = AMBIENT_TEMPERATURE_NULL_VALUE;
  static double bme280_temperature_shift = BME280_AMBIENT_TEMPERATURE_SHIFT;

  #ifndef BME280_HUMIDITY_SHIFT
    #define BME280_HUMIDITY_SHIFT 0
  #endif

  static double bme280_humidity       = HUMIDITY_NULL_VALUE;
  static double bme280_humidity_shift = BME280_HUMIDITY_SHIFT;

  #ifndef BME280_PRESSURE_SHIFT
    #define BME280_PRESSURE_SHIFT 0
  #endif

  static double bme280_pressure       = PRESSURE_NULL_VALUE;
  static double bme280_pressure_shift = BME280_PRESSURE_SHIFT;

#endif   // BME280

// SHT-3X (30, 31, 35, 85)
#if EXTERNAL_SENSOR_0 == EXTERNAL_SENSOR_SHT3X || \
    EXTERNAL_SENSOR_1 == EXTERNAL_SENSOR_SHT3X || \
    EXTERNAL_SENSOR_2 == EXTERNAL_SENSOR_SHT3X

  #define HAVE_SHT3X
  #define HAVE_SLOW_SENSOR

  #ifndef HAVE_AMBIENT_SENSOR
    #define HAVE_AMBIENT_SENSOR
  #endif

  #ifndef HAVE_HUMIDITY_SENSOR
    #define HAVE_HUMIDITY_SENSOR
  #endif

  // https://github.com/Sensirion/arduino-sht
  #include <SHTSensor.h>

  static SHTSensor sht3x_sensor;

  #ifndef SHT3X_AMBIENT_TEMPERATURE_SHIFT
    #define SHT3X_AMBIENT_TEMPERATURE_SHIFT 0
  #endif

  static double sht3x_temperature       = AMBIENT_TEMPERATURE_NULL_VALUE;
  static double sht3x_temperature_shift = SHT3X_AMBIENT_TEMPERATURE_SHIFT;

  #ifndef SHT3X_HUMIDITY_SHIFT
    #define SHT3X_HUMIDITY_SHIFT 0
  #endif

  static double sht3x_humidity       = HUMIDITY_NULL_VALUE;
  static double sht3x_humidity_shift = SHT3X_HUMIDITY_SHIFT;

#endif   // SHT-3X

//----------------------------------------------------------------------------------------------
// extra checks
//----------------------------------------------------------------------------------------------

#ifdef FAN_SPEED_NOISE_HUMIDITY
  #ifndef HAVE_HUMIDITY_SENSOR
    #error "FAN_SPEED_NOISE_HUMIDITY defined but you have not humidity sensor"
  #endif
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

#ifdef HAVE_SLOW_SENSOR
  // timer for slow sensors
  static SoftTimer slow_sensor_timer;

  #ifndef SLOW_SENSOR_UPDATE_INTERVAL
    // interval for slow sensor timer, ms
    #define SLOW_SENSOR_UPDATE_INTERVAL 15000
  #endif
#endif

//----------------------------------------------------------------------------------------------

bool sensorSetup() {

  bool result = true;

  #ifdef HAVE_DS18B20
    ds18b20_sensors.begin();

    // the default resolution at power-up is 12-bit.
    // 9 / 10 / 11 / 12 bit => 0.5 / 0.25 / 0.125 / 0.0625°C
    if (ds18b20_sensors.getAddress(ds18b20_water_address, DS18B20_WATER_TEMPERATURE_SENSOR_INDEX) == false) {
      result = false;
    } else {
      ds18b20_sensors.setResolution(ds18b20_water_address, 12);
    }
  #endif   // HAVE_DS18B20

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
  #endif   // HAVE_MLX90614

  #ifdef HAVE_BME280
    if (bme280_sensor.begin(BME280_ADDRESS_ALTERNATE) == false) {
      result = false;
    } else {
      // https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
      bme280_sensor.setSampling(
          Adafruit_BME280::MODE_NORMAL,
          Adafruit_BME280::SAMPLING_X1,   // temperature
          Adafruit_BME280::SAMPLING_X1,   // pressure
          Adafruit_BME280::SAMPLING_X1,   // humidity
          Adafruit_BME280::FILTER_X4,
          Adafruit_BME280::STANDBY_MS_1000
      );
    }
  #endif   // HAVE_BME280

  #ifdef HAVE_SHT3X
    if (sht3x_sensor.init() == false || sht3x_sensor.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH) == false) {
      result = false;
    }
  #endif   // HAVE_SHT3X

  #ifdef HAVE_FAST_SENSOR
    fast_sensor_timer.setTimeOutTime(FAST_SENSOR_UPDATE_INTERVAL);
    fast_sensor_timer.reset();
  #endif

  #ifdef HAVE_STANDARD_SENSOR
    standard_sensor_timer.setTimeOutTime(STANDARD_SENSOR_UPDATE_INTERVAL);
    standard_sensor_timer.reset();
  #endif

  #ifdef HAVE_SLOW_SENSOR
    slow_sensor_timer.setTimeOutTime(SLOW_SENSOR_UPDATE_INTERVAL);
    slow_sensor_timer.reset();
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

#ifdef HAVE_SLOW_SENSOR
void updateSlowSensors() {
  #ifdef HAVE_BME280
    // TODO: sometimes strange values - test for NaN?
    bme280_temperature = bme280_sensor.readTemperature() + bme280_temperature_shift;
    if (bme280_temperature < AMBIENT_TEMPERATURE_MIN_VALUE || bme280_temperature > AMBIENT_TEMPERATURE_MAX_VALUE) {
      bme280_temperature = AMBIENT_TEMPERATURE_NULL_VALUE;
    }

    bme280_humidity = bme280_sensor.readHumidity() + bme280_humidity_shift;
    if (bme280_humidity < HUMIDITY_MIN_VALUE || bme280_humidity > HUMIDITY_MAX_VALUE) {
      bme280_humidity = HUMIDITY_NULL_VALUE;
    }

    bme280_pressure = bme280_sensor.readPressure() + bme280_pressure_shift;
    if (bme280_pressure < PRESSURE_MIN_VALUE || bme280_pressure > PRESSURE_MAX_VALUE) {
      bme280_pressure = PRESSURE_NULL_VALUE;
    }
  #endif   // HAVE_BME280

  #ifdef HAVE_SHT3X
    if (sht3x_sensor.readSample() != false) {
      sht3x_temperature = sht3x_sensor.getTemperature() + sht3x_temperature_shift;
      if (sht3x_temperature < AMBIENT_TEMPERATURE_MIN_VALUE || sht3x_temperature > AMBIENT_TEMPERATURE_MAX_VALUE) {
        sht3x_temperature = AMBIENT_TEMPERATURE_NULL_VALUE;
      }

      sht3x_humidity = sht3x_sensor.getHumidity() + sht3x_humidity_shift;
      if (sht3x_humidity < HUMIDITY_MIN_VALUE || sht3x_humidity > HUMIDITY_MAX_VALUE) {
        sht3x_humidity = HUMIDITY_NULL_VALUE;
      }
    }
  #endif   // HAVE_SHT3X
}
#endif   // HAVE_SLOW_SENSOR
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
  #ifdef HAVE_SLOW_SENSOR
    if (slow_sensor_timer.hasTimedOut() == true) {
      updateSlowSensors();
      slow_sensor_timer.reset();
    }
  #endif

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

bool haveSecondaryWaterTemperature() {
  #ifdef SECONDARY_WATER_TEMPERATURE_SENSOR
    return true;
  #else
    return false;
  #endif
}
//----------------------------------------------------------------------------------------------

double secondaryWaterTemperature() {
  #ifdef SECONDARY_WATER_TEMPERATURE_SENSOR
    #if SECONDARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_DS18B20
      return ds18b20_water_temperature_filtered;
    #elif SECONDARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_MLX90614
      return mlx90614_object_temperature_filtered;
    #else
      #error "Unsupported secondary water temperature sensor"
    #endif
  #else
    return WATER_TEMPERATURE_NULL_VALUE;
  #endif
}
//----------------------------------------------------------------------------------------------

bool haveAmbientTemperature() {
  #ifdef HAVE_AMBIENT_SENSOR
    return true;
  #else
    return false;
  #endif
}
//----------------------------------------------------------------------------------------------

double ambientTemperature() {
  #if defined(HAVE_SHT3X)
    return sht3x_temperature;
  #elif defined(HAVE_BME280)
    return bme280_temperature;
  #elif defined(HAVE_MLX90614)
    return mlx90614_ambient_temperature;
  #else
    return AMBIENT_TEMPERATURE_NULL_VALUE;
  #endif
}
//----------------------------------------------------------------------------------------------

bool haveHumidity() {
  #ifdef HAVE_HUMIDITY_SENSOR
    return true;
  #else
    return false;
  #endif
}
//----------------------------------------------------------------------------------------------

double humidity() {
  #if defined(HAVE_SHT3X)
    return sht3x_humidity;
  #elif defined(HAVE_BME280)
    return bme280_humidity;
  #else
    return HUMIDITY_NULL_VALUE;
  #endif
}
//----------------------------------------------------------------------------------------------

bool havePressure() {
  #ifdef HAVE_PRESSURE_SENSOR
    return true;
  #else
    return false;
  #endif
}
//----------------------------------------------------------------------------------------------

double pressure() {
  #ifdef HAVE_BME280
    return bme280_pressure;
  #else
    return PRESSURE_NULL_VALUE;
  #endif
}

//----------------------------------------------------------------------------------------------
#ifdef AQUA_FAN_MASTER
//----------------------------------------------------------------------------------------------

void serializeSensors(JsonArray& json_sensors) {
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

  #ifdef HAVE_BME280
    if (bme280_temperature != AMBIENT_TEMPERATURE_NULL_VALUE) {
      JsonObject json_sensor = json_sensors.createNestedObject();
      json_sensor["sensor"]  = "BME280";
      json_sensor["type"]    = "ambient";
      json_sensor["value"]   = bme280_temperature;
      json_sensor["unit"]    = "C";
      json_sensor["role"]    = "external";
    }

    if (bme280_humidity != HUMIDITY_NULL_VALUE) {
      JsonObject json_sensor = json_sensors.createNestedObject();
      json_sensor["sensor"]  = "BME280";
      json_sensor["type"]    = "humidity";
      json_sensor["value"]   = bme280_humidity;
      json_sensor["unit"]    = "%";
      json_sensor["role"]    = "external";
    }

    if (bme280_pressure != PRESSURE_NULL_VALUE) {
      JsonObject json_sensor = json_sensors.createNestedObject();
      json_sensor["sensor"]  = "BME280";
      json_sensor["type"]    = "pressure";
      json_sensor["value"]   = bme280_pressure;
      json_sensor["unit"]    = "Pa";
      json_sensor["mmhg"]    = (bme280_pressure == PRESSURE_NULL_VALUE ? -1 : word(bme280_pressure / PRESSURE_PA_IN_MMHG));
      json_sensor["role"]    = "external";
    }
  #endif   // HAVE_BME280

  #ifdef HAVE_SHT3X
    if (sht3x_temperature != AMBIENT_TEMPERATURE_NULL_VALUE) {
      JsonObject json_sensor = json_sensors.createNestedObject();
      json_sensor["sensor"]  = "SHT3X";
      json_sensor["type"]    = "ambient";
      json_sensor["value"]   = sht3x_temperature;
      json_sensor["unit"]    = "C";
      json_sensor["role"]    = "external";
    }

    if (sht3x_humidity != HUMIDITY_NULL_VALUE) {
      JsonObject json_sensor = json_sensors.createNestedObject();
      json_sensor["sensor"]  = "SHT3X";
      json_sensor["type"]    = "humidity";
      json_sensor["value"]   = sht3x_humidity;
      json_sensor["unit"]    = "%";
      json_sensor["role"]    = "external";
    }
  #endif   // HAVE_SHT3X
}
//----------------------------------------------------------------------------------------------
#endif   // AQUA_FAN_MASTER
//----------------------------------------------------------------------------------------------
#endif   // !AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
