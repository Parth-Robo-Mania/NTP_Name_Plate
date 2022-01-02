#ifndef VARIABLE_DECLARATION_H
#define VARIABLE_DECLARATION_H

/* -------------------------------------------------------------------------- */
/*                            EEPROM CONFIGURATION                            */
/* -------------------------------------------------------------------------- */
#include "EEPROM.h"

/* -------------------------------------------------------------------------- */
/*                             WIFI CONFIGURATION                             */
/* -------------------------------------------------------------------------- */
// Wifi Parameters
#include <WiFi.h>
char ssid[50];
char password[50];
#define MAX_WIFI_CONNECT_ATTEMPT_TIME 10 // Max time in seconds to wait for WiFi Connection
#define NUMBER_OF_RETRIES 100            // Number of retries before opening configuration page

/* -------------------------------------------------------------------------- */
/*                             TIME CONFIGURATION                             */
/* -------------------------------------------------------------------------- */
// Reference (NTP Setup): https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/
#include "time.h"
int currentHour;   // Current Time in Hour
int currentMinute; // Current Time in Minute
const char *ntpServer = "asia.pool.ntp.org";
const long gmtOffset_sec = 19800; // +5:30 = (5*60*60) + (30*60) = 19800
const int daylightOffset_sec = 0; // India doesn't observe DayLight Saving

/* -------------------------------------------------------------------------- */
/*                           WEBSERVER CONFIGURATION                          */
/* -------------------------------------------------------------------------- */
// Reference (ESP32 Hotspot): https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/
// Reference (ESP32 Server): https://randomnerdtutorials.com/esp32-esp8266-input-data-html-form/
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "HTML_PAGE.h"
AsyncWebServer server(80);
const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "password";
const char *PARAM_INPUT_3 = "start_hour";
const char *PARAM_INPUT_4 = "stop_hour";
const char *PARAM_INPUT_5 = "start_minutes";
const char *PARAM_INPUT_6 = "stop_minutes";

/* -------------------------------------------------------------------------- */
/*                         CONTROL TIME CONFIGURATION                         */
/* -------------------------------------------------------------------------- */
int StartHour;                           // Variable to define the Light Start Time
int StopHour;                            // Variable to define the Light Stop Time
int StartMinute;                         // Variable to define the Light Start Time
int StopMinute;                          // Variable to define the Light Stop Time
bool lightStatus = LOW;                  // Current Status of Light Relay
uint32_t start_time;                     // Time in minutes: (StartHour*60) + StartMinutes
uint32_t stop_time;                      // Time in minutes: (StopHour*60) + StopMinutes
uint32_t current_time;                   // Time in minutes: (currentHour*60) + currentMinutes
#define TIME_AT_TWELVE_IN_MINS 720       // Mins
#define TIME_AT_TWENTY_FOUR_IN_MINS 1440 // Mins
bool interrupt_status = false;           // true = Interrupt Detected, false = No Interrupt

#endif