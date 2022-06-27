#include "button.h"
//----------------------------------------------------------------------------------------------
#ifndef AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------
// TTP223
#ifdef AQUA_FAN_MASTER
  #ifndef BUTTON_MODE_PIN
    #define BUTTON_MODE_PIN D5
  #endif
  #ifndef BUTTON_DOWN_PIN
    #define BUTTON_DOWN_PIN D6
  #endif
  #ifndef BUTTON_UP_PIN
    #define BUTTON_UP_PIN   D7
  #endif
#elif defined(AQUA_FAN_STANDALONE)
  #ifndef BUTTON_MODE_PIN
    #define BUTTON_MODE_PIN A0
  #endif
  #ifndef BUTTON_DOWN_PIN
    #define BUTTON_DOWN_PIN A1
  #endif
  #ifndef BUTTON_UP_PIN
    #define BUTTON_UP_PIN   A2
  #endif
#else
  #error "Something wrong with buttons"
#endif
//----------------------------------------------------------------------------------------------
static onButtonCallback on_button_up   = NULL;
static onButtonCallback on_button_down = NULL;
static onButtonCallback on_button_mode = NULL;
//----------------------------------------------------------------------------------------------

void onButtonUpCallback(onButtonCallback callback) {
  on_button_up = callback;
}
//----------------------------------------------------------------------------------------------

void onButtonDownCallback(onButtonCallback callback) {
  on_button_down = callback;
}
//----------------------------------------------------------------------------------------------

void onButtonModeCallback(onButtonCallback callback) {
  on_button_mode = callback;
}
//----------------------------------------------------------------------------------------------

bool buttonSetup() {
  pinMode(BUTTON_UP_PIN,   INPUT);
  pinMode(BUTTON_DOWN_PIN, INPUT);
  pinMode(BUTTON_MODE_PIN, INPUT);

  return true;
}
//----------------------------------------------------------------------------------------------

void buttonLoop() {
  if (digitalRead(BUTTON_MODE_PIN) == HIGH) {
    if (on_button_mode != NULL) {
      on_button_mode();
    }
  } else if (digitalRead(BUTTON_UP_PIN) == HIGH) {
    if (on_button_up != NULL) {
      on_button_up();
    }
  } else if (digitalRead(BUTTON_DOWN_PIN) == HIGH) {
    if (on_button_down != NULL) {
      on_button_down();
    }
  }
}
//----------------------------------------------------------------------------------------------
#endif   // !AQUA_FAN_SLAVE
