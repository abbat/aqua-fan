#include "wifi.h"
//----------------------------------------------------------------------------------------------
#ifdef AQUA_FAN_MASTER
//----------------------------------------------------------------------------------------------
#include "sensor.h"
#include "eeprom.h"
#include "controller.h"
//----------------------------------------------------------------------------------------------
// https://arduinojson.org/v6/api/
#include <ArduinoJson.h>
// https://arduino-esp8266.readthedocs.io/en/latest/
#include <ESP8266WiFi.h>
#include <AddrList.h>
// https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA
#include <ArduinoOTA.h>
// https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer
#include <ESP8266WebServer.h>
//----------------------------------------------------------------------------------------------
#ifndef HOSTNAME
  #define HOSTNAME "aqua-fan"
#endif
//----------------------------------------------------------------------------------------------
#ifndef NTP_SERVER_0
  #define NTP_SERVER_0 "0.ru.pool.ntp.org"
#endif
#ifndef NTP_SERVER_1
  #define NTP_SERVER_1 "1.ru.pool.ntp.org"
#endif
#ifndef NTP_SERVER_2
  #define NTP_SERVER_2 "2.ru.pool.ntp.org"
#endif
#ifndef NTP_TIMEZONE
  #define NTP_TIMEZONE "UTC"
#endif
//----------------------------------------------------------------------------------------------
static ESP8266WebServer web_server(80);
//----------------------------------------------------------------------------------------------

void http404() {
  web_server.send(404, F("text/plain"), F("Not Found"));
}
//----------------------------------------------------------------------------------------------

void httpStatus() {
  // https://arduinojson.org/v6/assistant/
  DynamicJsonDocument json(2048);

  JsonObject json_fans = json.createNestedObject("fans");
  serializeFans(json_fans);

  JsonArray json_sensors = json.createNestedArray("sensors");
  serializeSensors(json_sensors);

  time_t t = time(NULL);
  if (t != -1) {
    struct tm tm;
    if (localtime_r(&t, &tm) != NULL) {
      char ts[21];
      if (snprintf(ts, 21, "%d-%.2d-%.2dT%.2d:%.2d:%.2dZ", 1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec) > 0) {
        json["ts"] = ts;
      } else {
        json["ts"] = "1900-01-01T00:00:02Z";
      }
    } else {
      json["ts"] = "1900-01-01T00:00:01Z";
    }
  } else {
    json["ts"] = "1900-01-01T00:00:00Z";
  }

  // TODO: fix uptime overflow
  json["uptime"] = (unsigned long)(millis() / 1000);

  JsonArray json_ips = json.createNestedArray("ips");
  for (auto itf: addrList) {
    if (itf.addr().isLocal() == false) {
      json_ips.add(itf.addr().toString());
    }
  }

  String out;
  serializeJson(json, out);

  web_server.send(200, F("application/json; charset=utf-8"), out + "\n");
}
//----------------------------------------------------------------------------------------------
// experimental
void httpSettings() {
  if (web_server.method() != HTTP_POST) {
    web_server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  for (uint8_t i = 0; i < web_server.args(); i++) {
    const String& key = web_server.argName(i);
    const String& val = web_server.arg(i);

    // experimental
    if (key == "water-temperature-base") {
      // TODO: strict check value
      // TODO: accuracy convert to x10 integer value
      double base = val.toDouble();
      if (setWaterTemperatureBase10(base * 10.0) == true) {
        eepromCommit();
      } else {
        // TODO: send water temperature base valid range
        web_server.send(412, "text/plain", "invalid water-temperature-base value");
        return;
      }
    } else if (key == "min-fan-speed") {
      // TODO: strict check value
      long speed = val.toInt();
      if (setMinFanSpeed(speed) == true) {
        eepromCommit();
      } else {
        web_server.send(412, "text/plain", "min-fan-speed must be 0-100 and lower or equal max-fan-speed");
        return;
      }
    } else if (key == "max-fan-speed") {
      // TODO: strict check value
      long speed = val.toInt();
      if (setMaxFanSpeed(speed) == true) {
        eepromCommit();
      } else {
        web_server.send(412, "text/plain", "max-fan-speed must be 0-100 and greather or equal min-fan-speed");
        return;
      }
    }
  }

  web_server.send(200, "text/plain", "");
}
//----------------------------------------------------------------------------------------------

bool wifiSetup() {
  // init wifi
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // setup local time
  configTime(NTP_TIMEZONE, NTP_SERVER_0, NTP_SERVER_1, NTP_SERVER_2);

  // init webserver
  web_server.on("/status",   httpStatus);
  web_server.on("/settings", httpSettings);
  web_server.onNotFound(http404);
  web_server.begin();

  // init on air update
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPort(8266);
  ArduinoOTA.begin();

  return true;
}
//----------------------------------------------------------------------------------------------

void wifiLoop() {
  if (WiFi.isConnected() == true) {
    web_server.handleClient();
    ArduinoOTA.handle();
  }
}
//----------------------------------------------------------------------------------------------
#endif   // AQUA_FAN_MASTER
//----------------------------------------------------------------------------------------------
