#ifndef _AQUA_FAN_FAN_H_
#define _AQUA_FAN_FAN_H_
//----------------------------------------------------------------------------------------------
#include "aqua-fan.h"
//----------------------------------------------------------------------------------------------
#if defined(AQUA_FAN_STANDALONE) || defined(AQUA_FAN_SLAVE)

  // fan pwm pins (only D3, D9 and D10 can be used)
  #define FAN0_PWM 3
  #define FAN1_PWM 9
  #define FAN2_PWM 10

  // fan tac pins (different PCINT recommended)
  // - PCINT0 - D8, D11, D12
  // - PCINT2 - D4, D6, D7
  // - PCINT1 - A0, A1, A2, A3
  #define FAN0_TAC 4
  #define FAN1_TAC 8
  #define FAN2_TAC 11

  // fan pwm frequency, Hz (25KHz recommended, can be from 21kHz to 28KHz)
  #ifndef FAN_PWM_FREQ
    #define FAN_PWM_FREQ 25000
  #endif

#endif   // AQUA_FAN_SINGLE || AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
// return fan count or -1 on error
byte fanCount();

// return rpm for i-th fan or -1 on error
word fanRPM(byte i);

// set fan pwm for i-th fan, return false on error
bool fanPWM(byte i, byte pwm);
//----------------------------------------------------------------------------------------------
bool fanSetup();
void fanLoop();
//----------------------------------------------------------------------------------------------
#endif   // _AQUA_FAN_FAN_H_
