//!========================== PRIMARY INCLUDES ==========================//
#include <Arduino.h>
#include "PIN_CONFIGURATION.h"
#include "HTML_PAGE.h"
#include <string>
//!===========================================================================//

//!========================== EEPROM CONFIGURATIONS ==========================//
#include <EEPROM.h>
#define SSID_SIZE_LOCATION 0
#define PASSWORD_SIZE_LOCATION 1
#define SAVED_START_HOUR_LOCATION 2
#define SAVED_START_MINUTES_LOCATION 3
#define SAVED_STOP_HOUR_LOCATION 4
#define SAVED_STOP_MINUTES_LOCATION 5
#define SSID_SAVE_LOCATION 10
#define PASSWORD_SAVE_LOCATION 50
//!===========================================================================//

//!========================== WIFI CONFIGURATIONS ==========================//
// Wifi Parameters
char ssid[50];
char password[50];
//!===========================================================================//

//!========================== NTP CONFIGURATIONS ==========================//
#include <NTPClient.h>
//!===========================================================================//

//!========================== WEBSERVER CONFIGURATIONS ==========================//
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
AsyncWebServer server(80); // Object of WebServer(HTTP port, 80 is defult)
const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "password";
const char *PARAM_INPUT_3 = "start_hour";
const char *PARAM_INPUT_4 = "stop_hour";
const char *PARAM_INPUT_5 = "start_minutes";
const char *PARAM_INPUT_6 = "stop_minutes";
//!===========================================================================//

//!========================== EEPROM CONFIGURATIONS ==========================//
//!===========================================================================//
#include <WiFiUdp.h>

// Variable Definition
int StartHour;                                        // Variable to define the Light Start Time
int StopHour;                                         // Variable to define the Light Stop Time
int StartMinute;                                      // Variable to define the Light Start Time
int StopMinute;                                       // Variable to define the Light Stop Time
int currentHour;                                      // Current Time in Hour
int currentMinute;                                    // Current Time in Minute
int currentSecond;                                    // Current Time in Second
int currentHour1;                                     // Current Time in Hour for verification
const float hourToMinuteConversion = 60U;             // 1hr * 60min
const float minuteToSecondConversion = 60U;           // 1min * 60sec
const float secondToMicroSecondConversion = 1000000U; // 1sec * 1000000uSec
const float secondToMillisecondConversion = 1000U;    // 1sec * 1000ms
bool lightStatus = LOW;                               // Current Status of Light Relay
float turnOnDelay;                                    // Duration to keep Lamp on before rechecking Time
float sleepDuration;                                  // Variable to define the sleep duration

// Time Correction Parameters
const long utcOffsetInSeconds = 19800; //+5:30 For correct time in India

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Connecting to Wifi
void connectToWifi()
{
  WiFi.disconnect();
retryTime:
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(50);
    Serial.print(".");
  }
  Serial.println("\n WiFi connected to: " + String(ssid));
  timeClient.begin();
  timeClient.update();
  Serial.println("Test Time 1 = " + String(timeClient.getFormattedTime()));
  currentHour1 = timeClient.getHours();
  timeClient.update();
  Serial.println("Test Time 2 = " + String(timeClient.getFormattedTime()));
  currentHour = timeClient.getHours();
  if (currentHour != currentHour1)
  {
    Serial.println("Time Error !!! Resyncing Time.....");
    goto retryTime;
  }
  Serial.println("Time Sync Successful");
  Serial.println("Uodated Time = " + String(timeClient.getFormattedTime()));
  currentHour = timeClient.getHours();     // Storing Current Time into Variable
  currentMinute = timeClient.getMinutes(); // Storing Current Time into Variable
  currentSecond = timeClient.getSeconds(); // Storing Current Time into Variable
  return;
}

void notFound(AsyncWebServerRequest *request)
{
  Serial.println("Cannot Create Server");
  request->send(404, "text/plain", "Not found");
}

void change_configuration_settings(void)
{
  Serial.println("Some Configurations are missing.\nSetting up Configuration Page");
  WiFi.disconnect();

  /* Configuring Webserver */
  char router_ssid[] = "Name_Plate";
  char router_pwd[] = "123456789";
  WiFi.softAP(router_ssid, router_pwd);
  IPAddress IP = WiFi.softAPIP();
  delay(100);

  Serial.println("Connect to:");
  Serial.println("SSID: " + (String)router_ssid);
  Serial.println("Password: " + (String)router_pwd);
  Serial.print("IP address: ");
  Serial.println(IP); // IP: 192.168.4.1

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String inputMessage;
    String inputParam;
    if (request->hasParam(PARAM_INPUT_1))
    {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      Serial.println("SSID: " + inputMessage);
      EEPROM.write(SSID_SIZE_LOCATION, 0);
      EEPROM.commit();
      int n = inputMessage.length();
      EEPROM.write(SSID_SIZE_LOCATION, n);
      for (size_t i = 0; i < n; i++)
        EEPROM.write(SSID_SAVE_LOCATION + i, inputMessage[i]);
      EEPROM.commit();
      delay(500);
    }

    if (request->hasParam(PARAM_INPUT_2))
    {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
      Serial.println("Password: " + inputMessage);
      EEPROM.write(PASSWORD_SIZE_LOCATION, 0);
      EEPROM.commit();
      int n = inputMessage.length();
      EEPROM.write(PASSWORD_SIZE_LOCATION, n);
      for (size_t i = 0; i < n; i++)
        EEPROM.write(PASSWORD_SAVE_LOCATION + i, inputMessage[i]);
      EEPROM.commit();
      delay(500);

    }

    if (request->hasParam(PARAM_INPUT_3))
    {
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
      char char_message[inputMessage.length()];
      strcpy(char_message, inputMessage.c_str());
      int message = atoi(char_message);
      EEPROM.write(SAVED_START_HOUR_LOCATION, message);
      Serial.println("Start Hour: " + (String)message);
      EEPROM.commit();
      delay(500);
    }

    if (request->hasParam(PARAM_INPUT_4))
    {
      inputMessage = request->getParam(PARAM_INPUT_4)->value();
      inputParam = PARAM_INPUT_4;
      char char_message[inputMessage.length()];
      strcpy(char_message, inputMessage.c_str());
      int message = atoi(char_message);
      EEPROM.write(SAVED_STOP_HOUR_LOCATION, message);
      Serial.println("Stop Hour: " + (String)message);
      EEPROM.commit();
      delay(500);
    }

    if (request->hasParam(PARAM_INPUT_5))
    {
      inputMessage = request->getParam(PARAM_INPUT_5)->value();
      inputParam = PARAM_INPUT_5;
      char char_message[inputMessage.length()];
      strcpy(char_message, inputMessage.c_str());
      int message = atoi(char_message);
      EEPROM.write(SAVED_START_MINUTES_LOCATION, message);
      Serial.println("Start Minutes: " + (String)message);
      EEPROM.commit();
      delay(500);
    }

    if (request->hasParam(PARAM_INPUT_6))
    {
      inputMessage = request->getParam(PARAM_INPUT_6)->value();
      inputParam = PARAM_INPUT_6;
      char char_message[inputMessage.length()];
      strcpy(char_message, inputMessage.c_str());
      int message = atoi(char_message);
      EEPROM.write(SAVED_STOP_MINUTES_LOCATION, message);
      Serial.println("Stop Minutes: " + (String)message);
      EEPROM.commit();
      delay(500);

    }

    else
    {
      inputMessage = "No message sent";
      inputParam = "none";
    } });
  server.onNotFound(notFound);
  server.begin();
  while (1)
  {
    yield();
  }
}

void read_configuration_details_from_eeprom(void)
{
  Serial.println("Reading Configuration Files");
  // Reading ssid and password length from eeprom
  int ssid_length = EEPROM.read(SSID_SIZE_LOCATION);
  int password_length = EEPROM.read(PASSWORD_SIZE_LOCATION);
  Serial.println("SSID Length: " + (String)ssid_length);
  Serial.println("Password Length: " + (String)password_length);

  // Reading the configurations from the eeprom
  for (size_t i = 0; i < ssid_length; i++)
    ssid[i] = EEPROM.read(SSID_SAVE_LOCATION + i);
  Serial.println("SSID: " + (String)ssid);
  for (size_t i = 0; i < password_length; i++)
    password[i] = EEPROM.read(PASSWORD_SAVE_LOCATION + i);
  Serial.println("Password: " + (String)password);
  StartHour = EEPROM.read(SAVED_START_HOUR_LOCATION);
  StartMinute = EEPROM.read(SAVED_START_MINUTES_LOCATION);
  Serial.println("Start Time: " + (String)StartHour + ":" + (String)StartMinute);
  StopMinute = EEPROM.read(SAVED_STOP_MINUTES_LOCATION);
  StopHour = EEPROM.read(SAVED_STOP_HOUR_LOCATION);
  Serial.println("Stop Time: " + (String)StopHour + ":" + (String)StopMinute);

  // checking if all parameters are correct or not
  if ((ssid_length == 0) || (password_length == 0) || (StartHour == StopHour))
    change_configuration_settings();
}

void basic_setup(void)
{
  Serial.println("Basic Setup");
  pinMode(LIGHT_RELAY, OUTPUT); // Setting Relay Pin to Output
  pinMode(INT_BUTTON, INPUT);   // Interrupt button to change configuration settings
  pinMode(LED_PIN, OUTPUT);     // LED Button to indicate whether you are in configuration setting or not
  EEPROM.begin(512);
  read_configuration_details_from_eeprom();
}

void setup()
{
  Serial.begin(115200); // Enabling Serial Port
  basic_setup();
  connectToWifi();
  Serial.println("ESP8266 Loop Start");
}

void loop()
{
  // decisionMakingMode(); // Decide on whether to turn ON/OFF Lights or to go to sleep
  connectToWifi();
}