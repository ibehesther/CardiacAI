#ifndef _MAIN_HEADER_
#define _MAIN_HEADER_
#include "esp32-hal-gpio.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#define WIFI_CREDS "wifi_info"
#define ECG_PIN 34
#define LO_PLUS 33
#define LO_MINUS 32


// Button to key in new wifi details
struct Button {
  const uint8_t PIN;
  bool pressed;
  unsigned long lastPressTime;
};
extern volatile Button resetButton; // reset wifi details

void isr();
void startLocalServer();
void stopLocalServer();

#endif // _MAIN_HEADER_
