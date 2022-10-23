#include "controller.h"
//----------------------------------------------------------------------------------------------
#ifndef AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
#include "fan.h"
#include "sensor.h"
#include "eeprom.h"
//----------------------------------------------------------------------------------------------
// https://github.com/end2endzone/SoftTimers
#include <SoftTimers.h>

#ifndef FAN_UPDATE_INTERVAL
  // fan update interval, ms
  // 2s is enougth for 95% rpm accuracy
  #define FAN_UPDATE_INTERVAL 2000
#endif

// timer for update fans
static SoftTimer fan_timer;

// last known fan speed, 1/%
static double fan_speed = 0;

#ifdef AQUA_FAN_MASTER
  // fan rotate start/stop time
  static unsigned long fan_uptime = 0;
#endif

// master or standalone fan status
typedef struct fan_s {
  byte pwm;     // current pwm
  byte lopwm;   // last known stop pwm
  byte hipwm;   // last known start pwm
  word rpm;     // last known rpm
  word hirpm;   // last known max rpm
  word base;    // required rpm
} fan_t;

static fan_t* fan       = NULL;
static byte   fan_count = 0;
//----------------------------------------------------------------------------------------------

void setPWM(byte i, byte pwm) {
  if (fan[i].pwm != pwm && fanPWM(i, pwm) == true) {
    fan[i].pwm = pwm;
  }
}
//----------------------------------------------------------------------------------------------

void updateRPM() {
  // recalibrate fans
  word srpm = 0;
  word hrpm = 0;
  for (byte i = 0; i < fan_count; i++) {
    word r = fan[i].rpm;
    fan[i].rpm = fanRPM(i);
    if (fan[i].rpm == (word)(-1)) {
        // error recovery
        fan[i].rpm = r;
        continue;
    }

    // change max fan rpm
    if (fan[i].hirpm < fan[i].rpm || fan[i].pwm == 255) {
      fan[i].hirpm = fan[i].rpm;
    }

    // if fan was in stopped state and rotated now, set new start pwm value
    if (r == 0 && fan[i].rpm > 0 && (fan[i].hipwm == 0 || fan[i].hipwm > fan[i].pwm)) {
      fan[i].hipwm = fan[i].pwm;
    }

    // if fan was in rotating state and stopped now, set new stop pwm value
    if (r > 0 && fan[i].rpm == 0 && fan[i].lopwm <= fan[i].pwm) {
      fan[i].lopwm = fan[i].pwm + 1;
    }

    srpm += fan[i].rpm;
    hrpm += fan[i].hirpm;
  }

  double fs;
  if (hrpm == 0) {
    fs = 0;
  } else {
    fs = (double)srpm / hrpm;
  }

  #ifdef AQUA_FAN_MASTER
    if ((fan_speed == 0 && fs != 0) || (fan_speed != 0 && fs == 0)) {
      fan_uptime = millis();
    }
  #endif

  fan_speed = fs;
}
//----------------------------------------------------------------------------------------------

void updateFans() {
  for (byte i = 0; i < fan_count; i++) {
    // if fan direction is to stop and it still not stopped, slow-down decrease rotation speed
    if (fan[i].base == 0) {
      if (fan[i].pwm != 0) {
        setPWM(i, fan[i].pwm - 1);
      }

      continue;
    }

    // 1 pwm ~ 7 rpm
    const word rpm_per_pwm = 7;

    // slow up / down speed to base rpm
    short dpwm;
    if (fan[i].rpm < fan[i].base && fan[i].base - fan[i].rpm > rpm_per_pwm) {
      if (fan[i].pwm == 0) {
        // start fan from stopped state
        dpwm = fan[i].hipwm;
      } else {
        dpwm = 1;
      }
    } else if (fan[i].rpm > fan[i].base && fan[i].rpm - fan[i].base > rpm_per_pwm) {
      dpwm = -1;
    } else {
      continue;
    }

    // new pwm
    short pwm = constrain(dpwm + fan[i].pwm, 0, 255);

    // disable stopping before reaching the temperature base
    if (pwm < fan[i].lopwm) {
      pwm = fan[i].lopwm;
    }

    setPWM(i, pwm);
  }
}
//----------------------------------------------------------------------------------------------
#if defined(FAN_SPEED_NOISE_MULTIPLER) || defined(FAN_SPEED_NOISE_HUMIDITY)
inline double log_base(double value, double base) {
  if (base == M_E) {
    return log(value);
  } else if (base == 10.0) {
    return log10(value);
  } else if (base == 1.0) {
    return value;
  } else if (base <= 0.0) {
    return 0.0;
  }

  return log(value) / log(base);
}
#endif   // FAN_SPEED_NOISE_MULTIPLER || FAN_SPEED_NOISE_HUMIDITY
//----------------------------------------------------------------------------------------------

void updateDirections()
{
  #ifndef WATER_TEMPERATURE_SENSOR_NOISE
    #if PRIMARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_MLX90614
      #define WATER_TEMPERATURE_SENSOR_NOISE 0.3
    #elif PRIMARY_WATER_TEMPERATURE_SENSOR == WATER_TEMPERATURE_SENSOR_DS18B20
      // 0.0625 * 1.5 = 0.09375 ~ 0.1
      #define WATER_TEMPERATURE_SENSOR_NOISE 0.1
    #else
      #error "Unsupported water temperature sensor"
    #endif
  #endif

  // lower bound of prescion hysteresis
  #ifndef WATER_TEMPERATURE_SENSOR_NOISE_LOWER
    #define WATER_TEMPERATURE_SENSOR_NOISE_LOWER WATER_TEMPERATURE_SENSOR_NOISE
  #endif

  // upper bound of prescion hysteresis
  #ifndef WATER_TEMPERATURE_SENSOR_NOISE_UPPER
    #define WATER_TEMPERATURE_SENSOR_NOISE_UPPER WATER_TEMPERATURE_SENSOR_NOISE
  #endif

  // temperature window for 0-100% of fan speed
  #ifndef FAN_SPEED_TEMPERATURE_WINDOW
    #define FAN_SPEED_TEMPERATURE_WINDOW 0.5
  #endif

  double water_temperature = primaryWaterTemperature();

  // actual and base water temperature diff
  double dt = water_temperature - waterTemperatureBase();
  if (dt <= -WATER_TEMPERATURE_SENSOR_NOISE_LOWER) {
    // current water temperature <= base water temperature - stop fans
    for (byte i = 0; i < fan_count; i++) {
      fan[i].base = 0;
    }
    return;
  } else if (dt <= WATER_TEMPERATURE_SENSOR_NOISE_UPPER && fan_speed == 0) {
    // disable fan start before prescion hysteresis excess
    return;
  }

  // fan speed model
  if (dt >= FAN_SPEED_TEMPERATURE_WINDOW) {
    // maximum speed if delta too high
    dt = 1.0;
  } else {
    // linear speed in temperature window
    dt = (dt + WATER_TEMPERATURE_SENSOR_NOISE_LOWER) / (FAN_SPEED_TEMPERATURE_WINDOW + WATER_TEMPERATURE_SENSOR_NOISE_LOWER);

    // logarithm speed in temperature window
    #if defined(FAN_SPEED_NOISE_HUMIDITY)
      double base = humidity();
      if (base == HUMIDITY_NULL_VALUE) {
        base = M_E;     // like ~27% humidity
      } else if (base < 1.0) {
        base = 0.1;     // like 1% humidity
      } else {
        base /= 10.0;   // map 1-100% humidity to 0.1-10 base
      }
      dt = log_base(dt * (base - 1.0) + 1.0, base);
    #elif defined(FAN_SPEED_NOISE_MULTIPLER)
      // FAN_SPEED_NOISE_MULTIPLER must be (0..1) for lower noise or (1..∞) for more agressive
      dt = log_base(dt * (FAN_SPEED_NOISE_MULTIPLER - 1.0) + 1.0, FAN_SPEED_NOISE_MULTIPLER);
    #endif
  }

  // limit rpm to range min/max fan speed
  const double max_fan_speed = maxFanSpeedF();
  if (dt > max_fan_speed) {
    dt = max_fan_speed;
  } else {
    const double min_fan_speed = minFanSpeedF();
    if (dt < min_fan_speed) {
      dt = min_fan_speed;
    }
  }

  // so, set fan speed base in rpm (but do not stop it)
  for (byte i = 0; i < fan_count; i++) {
    word base = dt * fan[i].hirpm;
    fan[i].base = (base == 0 ? 1 : base);
  }
}
//----------------------------------------------------------------------------------------------

bool controllerSetup() {
  fan_count = fanCount();
  if (fan_count != (byte)(-1)) {
    fan = (fan_t*)malloc(fan_count * sizeof(fan_t));
    if (fan != NULL) {
      for (byte i = 0; i < fan_count; i++) {
        memset(&fan[i], 0, sizeof(fan_t));
      }
    } else {
      // error!!!
      fan_count = 0;
    }
  } else {
    // error!!!
    fan_count = 0;
  }

  // set max fan speed for calibration
  for (byte i = 0; i < fan_count; i++) {
    fanPWM(i, 255);
  }

  // update max rpm
  for (byte i = 0; i < 2; i++) {
    delay(FAN_UPDATE_INTERVAL);
    updateRPM();
  }

  // stop fans
  for (byte i = 0; i < fan_count; i++) {
    fanPWM(i, 0);
  }

  // wait for fans stopped
  delay(2 * FAN_UPDATE_INTERVAL);

  // reset rpm
  updateRPM();

  // reset start / stop pwm
  for (byte i = 0; i < fan_count; i++) {
    fan[i].hipwm = 0;
    fan[i].lopwm = 0;
  }

  fan_timer.setTimeOutTime(FAN_UPDATE_INTERVAL);
  fan_timer.reset();

  return true;
}
//----------------------------------------------------------------------------------------------

void controllerLoop() {
  if (fan_timer.hasTimedOut() == true) {
    updateRPM();
    updateDirections();
    updateFans();

    fan_timer.reset();
  }
}
//----------------------------------------------------------------------------------------------

byte cachedFanCount() {
  return fan_count;
}
//----------------------------------------------------------------------------------------------

word cachedFanRPM(byte i) {
  if (i > fan_count) {
    return -1;
  }

  return fan[i].rpm;
}
//----------------------------------------------------------------------------------------------

double cachedFanSpeed() {
  return fan_speed;
}
//----------------------------------------------------------------------------------------------

#ifdef AQUA_FAN_MASTER
void serializeFans(JsonObject& json_fans) {
  JsonArray json_items = json_fans.createNestedArray("items");
  for (byte i = 0; i < fan_count; i++) {
    double fs = 0;
    if (fan[i].hirpm != 0) {
      fs = (double)fan[i].rpm / fan[i].hirpm * 100;
    }

    JsonObject json_fan = json_items.createNestedObject();
    json_fan["pwm"]   = fan[i].pwm;
    json_fan["lopwm"] = fan[i].lopwm;
    json_fan["hipwm"] = fan[i].hipwm;
    json_fan["rpm"]   = fan[i].rpm;
    json_fan["hirpm"] = fan[i].hirpm;
    json_fan["base"]  = fan[i].base;
    json_fan["speed"] = fs;
  }

  json_fans["speed"] = fan_speed * 100;

  long uptime = (millis() - fan_uptime) / 1000;
  json_fans["uptime"] = (fan_speed == 0 ? -uptime : uptime);
}
#endif   // AQUA_FAN_MASTER
//----------------------------------------------------------------------------------------------
#endif   // !AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
