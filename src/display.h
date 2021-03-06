#ifndef _AQUA_FAN_DISPLAY_H_
#define _AQUA_FAN_DISPLAY_H_
//----------------------------------------------------------------------------------------------
#include "aqua-fan.h"
//----------------------------------------------------------------------------------------------
#ifndef AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
#ifdef DISPLAY_TYPE
//----------------------------------------------------------------------------------------------
void displayClear();
bool displaySetup();
void displayLoop();
//----------------------------------------------------------------------------------------------
#endif   // DISPLAY_TYPE
//----------------------------------------------------------------------------------------------
#endif   // !AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
#endif   // _AQUA_FAN_DISPLAY_H_
