#include "fan.h"
//----------------------------------------------------------------------------------------------
#include <Wire.h>
//----------------------------------------------------------------------------------------------
//
// Fan standalone or slave board
//
//----------------------------------------------------------------------------------------------
#if defined(AQUA_FAN_STANDALONE) || defined(AQUA_FAN_SLAVE)
//----------------------------------------------------------------------------------------------
// for fan pwm
// https://github.com/GyverLibs/GyverPWM
#include <GyverPWM.h>
//----------------------------------------------------------------------------------------------
// for fan tac
// https://github.com/GreyGnome/EnableInterrupt
#define EI_NOTEXTERNAL   /* we do not use external interrupts */
#include <EnableInterrupt.h>
//----------------------------------------------------------------------------------------------

#if defined(FAN0_PWM) && defined(FAN1_PWM) && defined(FAN2_PWM)
  #define FAN_COUNT 3
#elif defined(FAN0_PWM) && defined(FAN1_PWM)
  #define FAN_COUNT 2
#elif defined(FAN0_PWM)
  #define FAN_COUNT 1
#else
  #error "Fan count must be from 1 to 3"
#endif

// slave or standalone single fan status
typedef struct fan_base_s {
  byte          pwm;   // current pwm
  volatile word tac;   // current tac counter
  unsigned long tick;  // reset tac millis()
} fan_base_t;

// fans status
static fan_base_t fan[FAN_COUNT];

//----------------------------------------------------------------------------------------------
// fans tac interrupt handlers
void fan0_tac() {
  fan[0].tac++;
}

#if FAN_COUNT > 1
void fan1_tac() {
  fan[1].tac++;
}

#if FAN_COUNT > 2
void fan2_tac() {
  fan[2].tac++;
}
#endif   // FAN_COUNT > 2
#endif   // FAN_COUNT > 1
//----------------------------------------------------------------------------------------------

byte fanPin(byte i) {
  byte pin;
  switch (i) {
    case 0: pin = FAN0_PWM; break;
  #if FAN_COUNT > 1
    case 1: pin = FAN1_PWM; break;
  #if FAN_COUNT > 2
    case 2: pin = FAN2_PWM; break;
  #endif   // FAN_COUNT > 2
  #endif   // FAN_COUNT > 1
    default:
      pin = -1;
  }

  return pin;
}
//----------------------------------------------------------------------------------------------

bool fanPWM(byte i, byte pwm) {
  byte pin = fanPin(i);
  if (pin == (byte)(-1)) {
    return false;
  }

  if (fan[i].pwm != pwm) {
    fan[i].pwm = pwm;
    PWM_set(pin, pwm);
  }

  return true;
}
//----------------------------------------------------------------------------------------------

word fanRPM(byte i) {
  if (i > FAN_COUNT) {
    return -1;
  }

  unsigned long tick = millis();
  if (tick - fan[i].tick == 0) {
    return -1;
  }

  word rpm = ((unsigned long)fan[i].tac * 60 * 1000) / ((tick - fan[i].tick) * 4);

  fan[i].tac  = 0;
  fan[i].tick = tick;

  return rpm;
}
//----------------------------------------------------------------------------------------------

bool fanStandaloneSetup() {
  // configure fans pwm
  for (byte i = 0; i < FAN_COUNT; i++) {
    memset(&fan[i], 0, sizeof(fan_base_s));

    byte pin = fanPin(i);

    pinMode(pin, OUTPUT);

    // FAST_PWM is more stable accross different timers
    PWM_frequency(pin, FAN_PWM_FREQ, FAST_PWM);

    // stop fans by default
    PWM_set(pin, 0);
  }

  // configure fans tac
  // one fan turn == 2 * RISING + 2 * FALLING == 4 * CHANGE
  // CHANGE mode is more stable (especially for low rpm)
  pinMode(FAN0_TAC, INPUT_PULLUP);
  enableInterrupt(FAN0_TAC, fan0_tac, CHANGE);

  #if FAN_COUNT > 1
    pinMode(FAN1_TAC, INPUT_PULLUP);
    enableInterrupt(FAN1_TAC, fan1_tac, CHANGE);
  #if FAN_COUNT > 2
    pinMode(FAN2_TAC, INPUT_PULLUP);
    enableInterrupt(FAN2_TAC, fan2_tac, CHANGE);
  #endif   // FAN_COUNT > 2
  #endif   // FAN_COUNT > 1

  for (byte i = 0; i < FAN_COUNT; i++) {
    fan[i].tick = millis();
  }

  return true;
}
//----------------------------------------------------------------------------------------------

void fanStandaloneLoop() {
  // nothing to do, just rotate
}
//----------------------------------------------------------------------------------------------

byte fanCount() {
  return FAN_COUNT;
}
//----------------------------------------------------------------------------------------------
#endif   // AQUA_FAN_STANDALONE || AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
//
// Fan i2c common
//
//----------------------------------------------------------------------------------------------
#ifndef AQUA_FAN_STANDALONE
//----------------------------------------------------------------------------------------------

// fan board i2c address
#ifndef FAN_I2C_ADDRESS
  #define FAN_I2C_ADDRESS 0x08
#endif

// return fan count, out - byte
#define FAN_I2C_CMD_COUNT 0x11
// return rpm for all fans, out - word[FAN_COUNT] (one rpm value for each fan)
#define FAN_I2C_CMD_RPM   0x22
// set pwm for all fans, in - byte[FAN_COUNT] (one pwm value for each fan)
#define FAN_I2C_CMD_PWM   0x33
// return rpm for one fan, in - byte (fan index), out - word (rpm)
#define FAN_I2C_CMD_RPM_I 0x44
// set pwm for one fan, in - byte (fan index), byte (pwm)
#define FAN_I2C_CMD_PWM_I 0x55
// return fan count and rpm for all fans, out - byte (fan count), word[FAN_COUNT] (one rpm value for each fan)
#define FAN_I2C_CMD_RPM_N 0x66
// set pwm for N fans, in - byte (fan count), word[fan_count] (pwm)
#define FAN_I2C_CMD_PWM_N 0x77

//----------------------------------------------------------------------------------------------
#endif   // !AQUA_FAN_STANDALONE
//----------------------------------------------------------------------------------------------
//
// Fan i2c slave
//
//----------------------------------------------------------------------------------------------
#ifdef AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------

// i2c request buffer
static byte i2c_data[sizeof(byte) + sizeof(word) * FAN_COUNT];

// i2c request size
static byte i2c_size = 0;

//----------------------------------------------------------------------------------------------

void fanSlaveRequest() {
  Wire.write(i2c_data, i2c_size);
  i2c_size = 0;
}
//----------------------------------------------------------------------------------------------

void fanSlaveReceive(int count) {
  while (Wire.available() > 0) {
    byte cmd = Wire.read();
    switch (cmd) {

      case FAN_I2C_CMD_COUNT: {
        i2c_size    = sizeof(byte);
        i2c_data[0] = FAN_COUNT;
      } break;

      case FAN_I2C_CMD_RPM: {
        i2c_size = sizeof(word) * FAN_COUNT;
        for (byte i = 0; i < FAN_COUNT; i++) {
          word rpm = fanRPM(i);
          byte off = i * sizeof(word);
          i2c_data[off + 0] = highByte(rpm);
          i2c_data[off + 1] = lowByte(rpm);
        }
      } break;

      case FAN_I2C_CMD_PWM: {
        for (byte i = 0; i < FAN_COUNT; i++) {
          if (Wire.available() > 0) {
            fanPWM(i, Wire.read());
          } else {
            break;
          }
        }
      } break;

      case FAN_I2C_CMD_RPM_I: {
        if (Wire.available() > 0) {
          i2c_size = sizeof(word);
          word rpm = fanRPM(Wire.read());
          i2c_data[0] = highByte(rpm);
          i2c_data[1] = lowByte(rpm);
        }
      } break;

      case FAN_I2C_CMD_PWM_I: {
        if (Wire.available() > 0) {
          byte i = Wire.read();
          if (Wire.available() > 0) {
            fanPWM(i, Wire.read());
          }
        }
      } break;

      case FAN_I2C_CMD_RPM_N: {
        i2c_size    = sizeof(byte) + sizeof(word) * FAN_COUNT;
        i2c_data[0] = FAN_COUNT;
        for (byte i = 0; i < FAN_COUNT; i++) {
          word rpm = fanRPM(i);
          byte off = i * sizeof(word);
          i2c_data[off + 1] = highByte(rpm);
          i2c_data[off + 2] = lowByte(rpm);
        }
      } break;

      case FAN_I2C_CMD_PWM_N: {
        if (Wire.available() > 0) {
          byte n = Wire.read();
          for (byte i = 0; i < n; i++) {
            if (Wire.available() > 0) {
              fanPWM(i, Wire.read());
            } else {
              break;
            }
          }
        }
      } break;

      default:
        Wire.flush();
        // if Wire.flush() not implemented
        while (Wire.available() > 0) {
          Wire.read();
        }
    }
  }
}
//----------------------------------------------------------------------------------------------

bool fanSlaveSetup() {
  fanStandaloneSetup();

  // configure i2c
  Wire.begin(FAN_I2C_ADDRESS);
  // clock is set by master, I'm not sure what it needs to be set for high speed slave
  Wire.setClock(400000L);

  Wire.onReceive(fanSlaveReceive);
  Wire.onRequest(fanSlaveRequest);

  return true;
}
//----------------------------------------------------------------------------------------------

void fanSlaveLoop() {
  fanStandaloneLoop();
}
//----------------------------------------------------------------------------------------------
#endif   // AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
//
// Fan i2c master
//
//----------------------------------------------------------------------------------------------
#ifdef AQUA_FAN_MASTER
//----------------------------------------------------------------------------------------------

byte fanCount() {
  Wire.beginTransmission(FAN_I2C_ADDRESS);
  Wire.write(FAN_I2C_CMD_COUNT);
  if (Wire.endTransmission() == 0 && Wire.requestFrom(FAN_I2C_ADDRESS, 1) >= sizeof(byte)) {
    return Wire.read();
  }

  Wire.flush();
  return -1;
}
//----------------------------------------------------------------------------------------------

word fanRPM(byte i) {
  Wire.beginTransmission(FAN_I2C_ADDRESS);
  Wire.write(FAN_I2C_CMD_RPM_I);
  Wire.write(i);
  if (Wire.endTransmission() == 0 && Wire.requestFrom(FAN_I2C_ADDRESS, sizeof(word)) >= sizeof(word)) {
    const byte hi = Wire.read();
    const byte lo = Wire.read();
    return word(hi, lo);
  }

  Wire.flush();
  return -1;
}
//----------------------------------------------------------------------------------------------

bool fanPWM(byte i, byte pwm) {
  Wire.beginTransmission(FAN_I2C_ADDRESS);
  Wire.write(FAN_I2C_CMD_PWM_I);
  Wire.write(i);
  Wire.write(pwm);
  if (Wire.endTransmission() == 0) {
    return true;
  }

  Wire.flush();
  return false;
}
//----------------------------------------------------------------------------------------------

bool fanMasterSetup() {
  Wire.begin();

  // system became unstable with it :(
  //Wire.setClock(400000L);

  return true;
}
//----------------------------------------------------------------------------------------------

void fanMasterLoop() {
  // nothing to do, just rotate
}
//----------------------------------------------------------------------------------------------
#endif   // AQUA_FAN_MASTER
//----------------------------------------------------------------------------------------------

bool fanSetup() {
  #if defined(AQUA_FAN_STANDALONE)
    Wire.begin();
    Wire.setClock(400000L);
    return fanStandaloneSetup();
  #elif defined(AQUA_FAN_MASTER)
    return fanMasterSetup();
  #elif defined(AQUA_FAN_SLAVE)
    return fanSlaveSetup();
  #else
    return false;
  #endif
}
//----------------------------------------------------------------------------------------------

void fanLoop() {
  #if defined(AQUA_FAN_STANDALONE)
    return fanStandaloneLoop();
  #elif defined(AQUA_FAN_MASTER)
    return fanMasterLoop();
  #elif defined(AQUA_FAN_SLAVE)
    return fanSlaveLoop();
  #endif
}
//----------------------------------------------------------------------------------------------
