#include "fan.h"
//----------------------------------------------------------------------------------------------
#ifndef AQUA_FAN_SLAVE
  #include "wifi.h"
  #include "button.h"
  #include "sensor.h"
  #include "eeprom.h"
  #include "display.h"
  #include "controller.h"
#endif   // !AQUA_FAN_SLAVE
//----------------------------------------------------------------------------------------------

void setup() {
  // must be first
  if (fanSetup() == false) {
    // TODO: beep error!!!
  }

#ifndef AQUA_FAN_SLAVE
  if (displaySetup() == false) {
    // TODO: beep error!!!
  }

  if (sensorSetup() == false) {
    // TODO: display error!!!
  }

  if (eepromSetup() == false) {
    // TODO: display error!!!
  }

  // init controller
  if (controllerSetup() == false) {
    // TODO: display error!!!
  }

  if (buttonSetup() == false) {
    // TODO: beep error!!!
  }
#endif   // !AQUA_FAN_SLAVE

#ifdef AQUA_FAN_MASTER
  if (wifiSetup() == false) {
    // TODO: display error!!!
  }
#endif

#ifndef AQUA_FAN_SLAVE
  displayClear();
#endif   // !AQUA_FAN_SLAVE
}
//----------------------------------------------------------------------------------------------

void loop() {
  fanLoop();

#ifndef AQUA_FAN_SLAVE
  sensorLoop();
  controllerLoop();
  buttonLoop();
  displayLoop();
#endif   // !AQUA_FAN_SLAVE

#ifdef AQUA_FAN_MASTER
  wifiLoop();
#endif
}
//----------------------------------------------------------------------------------------------
