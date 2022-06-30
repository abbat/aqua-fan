#ifndef _AQUA_FAN_FAN_H_
#define _AQUA_FAN_FAN_H_
//----------------------------------------------------------------------------------------------
#include "aqua-fan.h"
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
