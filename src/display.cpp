#include "display.h"
//----------------------------------------------------------------------------------------------
#ifndef AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
#ifdef DISPLAY_TYPE
//----------------------------------------------------------------------------------------------
#include "button.h"
#include "sensor.h"
#include "eeprom.h"
#include "controller.h"
//----------------------------------------------------------------------------------------------
// https://github.com/end2endzone/SoftTimers
#include <SoftTimers.h>
//----------------------------------------------------------------------------------------------
// SSD1306
#if DISPLAY_TYPE == DISPLAY_SSD1306_128_64 || \
    DISPLAY_TYPE == DISPLAY_SSD1306_128_32
  // https://github.com/GyverLibs/GyverOLED
  #include <GyverOLED.h>
#endif
//----------------------------------------------------------------------------------------------
#ifndef DISPLAY_UPDATE_INTERVAL
  // display update interval, ms
  #define DISPLAY_UPDATE_INTERVAL 1000
#endif

#ifndef DISPLAY_UPDATE_EXTRA_INTERVAL
  // display update extra information interval, ms
  #define DISPLAY_UPDATE_EXTRA_INTERVAL 5000
#endif

// timer for update display
static SoftTimer display_timer;
// timer for update display from extra sensors
unsigned long display_extra_timer = 0;

// screen id constants
#define SCREEN_ID_DEFAULT                  0   /* default screen                */
#define SCREEN_ID_WATER_TEMPERATURE_BASE   1   /* setup base temperature screen */
#define SCREEN_ID_MAX_FAN_SPEED            2   /* setup max fan speed screen    */
#define SCREEN_ID_MIN_FAN_SPEED            3   /* setup min fan speed screen    */

// id of displayed screen
static byte screen_id = SCREEN_ID_DEFAULT;
// timer for reset screen to default
static SoftTimer screen_timer;

#if DISPLAY_TYPE == DISPLAY_SSD1306_128_64

  #define DISPLAY_TYPE_SSD1306

  #define DISPLAY_WIDTH  128
  #define DISPLAY_HEIGHT 64

  #define DISPLAY_SCALE_1_WIDTH  6
  #define DISPLAY_SCALE_1_HEIGHT 8

  #define DISPLAY_SCALE_2_WIDTH  12
  #define DISPLAY_SCALE_2_HEIGHT 16

  #define DISPLAY_SCALE_3_WIDTH  18
  #define DISPLAY_SCALE_3_HEIGHT 24

  #ifndef DISPLAY_ADDRESS
    // 0x3D - 128x64, 0x3C - 128x32, use I2C scan for "aliexpress" displays
    #define DISPLAY_ADDRESS 0x3C
  #endif

  static GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> display(DISPLAY_ADDRESS);

#else
  #error "Unsupported display type"
#endif   // DISPLAY_TYPE == DISPLAY_SSD1306_XXX_XX
//----------------------------------------------------------------------------------------------
typedef double (*sensorValueCallback)();
typedef void   (*displayUpdateCallback)(byte x, byte y, double value);

typedef struct sensor_display_s {
  sensorValueCallback   sensor_cb;
  displayUpdateCallback display_cb;
} sensor_display_t;

static byte              extra_sensor_count   = 0;
static sensor_display_t* extra_sensor_display = NULL;
//----------------------------------------------------------------------------------------------

void displayUpdateExtraTemperature(byte x, byte y, double value) {
  #ifdef DISPLAY_TYPE_SSD1306
    display.setCursorXY(x, y);
    if (value != AMBIENT_TEMPERATURE_NULL_VALUE) {
      if (value < 10) {
        display.print(F(" "));
      }
      display.print(value, 1);
      display.print(F("C"));
    } else {
      display.print(F("     "));
    }
  #endif   // DISPLAY_TYPE_SSD1306
}
//----------------------------------------------------------------------------------------------

void displayUpdateExtraHumidity(byte x, byte y, double value) {
  #ifdef DISPLAY_TYPE_SSD1306
    display.setCursorXY(x, y);
    if (value != HUMIDITY_NULL_VALUE) {
      byte h = value;
      if (h < 10) {
        display.print(F("   "));
      } else if (h < 100) {
        display.print(F("  "));
      }
      display.print(h);
      display.print(F("%"));
    } else {
      display.print(F("     "));
    }
  #endif   // DISPLAY_TYPE_SSD1306
}
//----------------------------------------------------------------------------------------------

void displayUpdateExtraPressure(byte x, byte y, double value) {
  #ifdef DISPLAY_TYPE_SSD1306
    display.setCursorXY(x, y);
    if (value != PRESSURE_NULL_VALUE) {
      word p = value / PRESSURE_PA_IN_MMHG;
      // pressure in mm/Hg always 3 digits (641 - 816 mm/Hg, 560 mm/Hg in hurricate center)
      display.print(p);
      display.print(F("mm"));
    } else {
      display.print(F("     "));
    }
  #endif   // DISPLAY_TYPE_SSD1306
}
//----------------------------------------------------------------------------------------------

void displayUpdate() {
  #ifdef DISPLAY_TYPE_SSD1306
    // primary water temperature (main information)
    display.setScale(3);
    display.setCursorXY(47, 8);

    double t = primaryWaterTemperature();
    if (t != WATER_TEMPERATURE_NULL_VALUE) {
      if (t < 10) {
        display.print(F(" "));
      }
      display.print(t, 1);
    } else {
      display.print(F("    "));
    }

    display.setScale(2);
    display.setCursorXY(65, 40);

    // fan speed (background information)
    byte fs = cachedFanSpeed() * 100;
    if (fs == 0) {
      display.print(F("^-^"));
    } else if (fs == 100) {
      display.print(F("max"));
    } else {
      if (fs < 10) {
        display.print(F(" "));
      }
      display.print(fs);
      display.print(F("%"));
    }

    // any (mostly useless) extra information
    // we have 4 row x 5 col area and two fan states (on/off)
    display.setScale(1);

    // rpm for each fan
    for (byte i = 0; i < cachedFanCount(); i++) {
      display.setCursorXY(6, 8 + i * 12);
      display.print(F("*"));

      word rpm = cachedFanRPM(i);
      if (rpm == 0) {
        display.print(F(" off"));
      } else {
        if (rpm < 10) {
          display.print(F("   "));
        } else if (rpm < 100) {
          display.print(F("  "));
        } else if (rpm < 1000) {
          display.print(F(" "));
        }
        display.print(rpm);
      }
    }

    // other information from extra sensors
    if (extra_sensor_count > 0) {
      byte x = 6;
      byte y = 48;

      static byte extra_sensor_index = 0;
      extra_sensor_display[extra_sensor_index].display_cb(x, y, extra_sensor_display[extra_sensor_index].sensor_cb());

      display_extra_timer += DISPLAY_UPDATE_INTERVAL;
      if (display_extra_timer >= DISPLAY_UPDATE_EXTRA_INTERVAL) {
        extra_sensor_index++;
        display_extra_timer = 0;
      }

      if (extra_sensor_index >= extra_sensor_count) {
        extra_sensor_index = 0;
      }
    }

    display.update();
  #endif   // DISPLAY_TYPE_SSD1306
}
//----------------------------------------------------------------------------------------------

void displayClear() {
  #ifdef DISPLAY_TYPE_SSD1306
    display.clear();
    display.rect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1, OLED_STROKE);
  #endif
}
//----------------------------------------------------------------------------------------------

void displayDefaultSetup() {
  screen_id = SCREEN_ID_DEFAULT;

  eepromCommit();

  displayClear();
  displayUpdate();

  display_timer.reset();
}
//----------------------------------------------------------------------------------------------

void displayTitleSetup(const __FlashStringHelper* title) {
  displayClear();

  #ifdef DISPLAY_TYPE_SSD1306
    display.setScale(1);
    display.setCursorXY((DISPLAY_WIDTH - strlen_P((const char*)title) * DISPLAY_SCALE_1_WIDTH) / 2, DISPLAY_SCALE_1_HEIGHT);
    display.print(title);
  #endif
}
//----------------------------------------------------------------------------------------------

void displayTemperatureSetup(word v10) {
  #ifdef DISPLAY_TYPE_SSD1306
    display.setScale(3);

    display.setCursorXY((DISPLAY_WIDTH - 4 * DISPLAY_SCALE_3_WIDTH) / 2, 28);
    display.print((double)v10 / 10.0, 1);

    display.update();
  #endif
}
//----------------------------------------------------------------------------------------------

void displayPercentSetup(byte v) {
  #ifdef DISPLAY_TYPE_SSD1306
    display.setScale(3);

    display.setCursorXY((DISPLAY_WIDTH - 3 * DISPLAY_SCALE_3_WIDTH) / 2, 28);

    if (v == 0) {
      display.print(F("^-^"));
    } else if (v == 100) {
      display.print(F("max"));
    } else {
      if (v < 10) {
        display.print(F(" "));
      }
      display.print(v);
      display.print(F("%"));
    }

    display.update();
  #endif
}
//----------------------------------------------------------------------------------------------

void displayWaterTemperatureBaseSetup() {
  displayTitleSetup(F("base temp, C"));
  displayTemperatureSetup(waterTemperatureBase10());
}
//----------------------------------------------------------------------------------------------

void displayWaterTemperatureBaseUp() {
  word v = waterTemperatureBase10() + 1;
  if (v > MAX_WATER_TEMPERATURE_BASE_10) {
    v = MAX_WATER_TEMPERATURE_BASE_10;
  }

  if (setWaterTemperatureBase10(v) == true) {
    displayTemperatureSetup(v);
  }
}
//----------------------------------------------------------------------------------------------

void displayWaterTemperatureBaseDown() {
  word v = waterTemperatureBase10() - 1;
  if (v < MIN_WATER_TEMPERATURE_BASE_10) {
    v = MIN_WATER_TEMPERATURE_BASE_10;
  }

  if (setWaterTemperatureBase10(v) == true) {
    displayTemperatureSetup(v);
  }
}
//----------------------------------------------------------------------------------------------

byte percentUp(byte v, byte from, byte to) {
  v++;
  if (v > to) {
    v = to;
  }

  return v;
}
//----------------------------------------------------------------------------------------------

byte percentDown(byte v, byte from, byte to) {
  v--;
  if (v < from) {
    v = from;
  }

  return v;
}
//----------------------------------------------------------------------------------------------

void displayMaxFanSpeedSetup() {
  displayTitleSetup(F("max fan speed"));
  displayPercentSetup(maxFanSpeed());
}
//----------------------------------------------------------------------------------------------

void displayMaxFanSpeedUp() {
  byte v = percentUp(maxFanSpeed(), minFanSpeed(), 100);
  if (setMaxFanSpeed(v) == true) {
    displayPercentSetup(v);
  }
}
//----------------------------------------------------------------------------------------------

void displayMaxFanSpeedDown() {
  byte v = percentDown(maxFanSpeed(), minFanSpeed(), 100);
  if (setMaxFanSpeed(v) == true) {
    displayPercentSetup(v);
  }
}
//----------------------------------------------------------------------------------------------

void displayMinFanSpeedSetup() {
  displayTitleSetup(F("min fan speed"));
  displayPercentSetup(minFanSpeed());
}
//----------------------------------------------------------------------------------------------

void displayMinFanSpeedUp() {
  byte v = percentUp(minFanSpeed(), 0, maxFanSpeed());
  if (setMinFanSpeed(v) == true) {
    displayPercentSetup(v);
  }
}
//----------------------------------------------------------------------------------------------

void displayMinFanSpeedDown() {
  byte v = percentDown(minFanSpeed(), 0, maxFanSpeed());
  if (setMinFanSpeed(v) == true) {
    displayPercentSetup(v);
  }
}
//----------------------------------------------------------------------------------------------

void onButtonUp() {
  switch (screen_id) {
    case SCREEN_ID_WATER_TEMPERATURE_BASE: displayWaterTemperatureBaseUp(); break;
    case SCREEN_ID_MAX_FAN_SPEED:          displayMaxFanSpeedUp();          break;
    case SCREEN_ID_MIN_FAN_SPEED:          displayMinFanSpeedUp();          break;
    default:
      return;
  }

  delay(100);
  screen_timer.reset();
}
//----------------------------------------------------------------------------------------------

void onButtonDown() {
  switch (screen_id) {
    case SCREEN_ID_WATER_TEMPERATURE_BASE: displayWaterTemperatureBaseDown(); break;
    case SCREEN_ID_MAX_FAN_SPEED:          displayMaxFanSpeedDown();          break;
    case SCREEN_ID_MIN_FAN_SPEED:          displayMinFanSpeedDown();          break;
    default:
      return;
  }

  delay(100);
  screen_timer.reset();
}
//----------------------------------------------------------------------------------------------

void onButtonMode() {
  screen_id += 1;

  switch (screen_id) {
    case SCREEN_ID_WATER_TEMPERATURE_BASE: displayWaterTemperatureBaseSetup(); break;
    case SCREEN_ID_MAX_FAN_SPEED:          displayMaxFanSpeedSetup();          break;
    case SCREEN_ID_MIN_FAN_SPEED:          displayMinFanSpeedSetup();          break;
    default:
      screen_id = SCREEN_ID_DEFAULT;
      onButtonMode();
      return;
  }

  delay(300);
  screen_timer.reset();
}
//----------------------------------------------------------------------------------------------

bool displaySetup() {
  #ifdef DISPLAY_TYPE_SSD1306
    display.init();

    display.clear();
    display.rect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1, OLED_STROKE);

    display.setScale(2);

    const char* msg = "SETUP";
    display.setCursorXY((DISPLAY_WIDTH - strlen(msg) * 12) / 2, (DISPLAY_HEIGHT - 16) / 2);
    display.print(msg);

    display.update();
  #endif   // DISPLAY_TYPE_SSD1306

  // callbacks to display information from extra sensors
  extra_sensor_count =
    (haveHumidity()           == true ? 1 : 0) +
    (havePressure()           == true ? 1 : 0) +
    (haveAmbientTemperature() == true ? 1 : 0);

  if (extra_sensor_count > 0) {
    extra_sensor_display = (sensor_display_t*)malloc(extra_sensor_count * sizeof(sensor_display_t));
    if (extra_sensor_display != NULL) {
      memset(extra_sensor_display, 0, extra_sensor_count * sizeof(sensor_display_t));

      byte extra_sensor_index = 0;
      if (haveAmbientTemperature() == true) {
        extra_sensor_display[extra_sensor_index].sensor_cb  = ambientTemperature;
        extra_sensor_display[extra_sensor_index].display_cb = displayUpdateExtraTemperature;
        extra_sensor_index++;
      }

      if (haveHumidity() == true) {
        extra_sensor_display[extra_sensor_index].sensor_cb  = humidity;
        extra_sensor_display[extra_sensor_index].display_cb = displayUpdateExtraHumidity;
        extra_sensor_index++;
      }

      if (havePressure() == true) {
        extra_sensor_display[extra_sensor_index].sensor_cb  = pressure;
        extra_sensor_display[extra_sensor_index].display_cb = displayUpdateExtraPressure;
        extra_sensor_index++;
      }
    } else {
      extra_sensor_count = 0;
    }
  }

  // button callbacks
  onButtonUpCallback(onButtonUp);
  onButtonDownCallback(onButtonDown);
  onButtonModeCallback(onButtonMode);

  // display update timers
  screen_timer.setTimeOutTime(5000);

  display_timer.setTimeOutTime(DISPLAY_UPDATE_INTERVAL);
  display_timer.reset();

  return true;
}
//----------------------------------------------------------------------------------------------

void displayLoop() {
  if (screen_id == 0) {
    if (display_timer.hasTimedOut() == true) {
      displayUpdate();
      display_timer.reset();
    }
  } else if (screen_timer.hasTimedOut() == true) {
    displayDefaultSetup();
  }
}
//----------------------------------------------------------------------------------------------
#endif   // DISPLAY_TYPE
//----------------------------------------------------------------------------------------------
#endif   // !AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
