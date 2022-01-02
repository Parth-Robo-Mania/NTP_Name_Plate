//!========================== PRIMARY INCLUDES ==========================//
#include <Arduino.h>
#include "PIN_CONFIGURATION.h"
#include "HTML_PAGE.h"
#include <string>
//!===========================================================================//

//!========================== EEPROM CONFIGURATIONS ==========================//
#include <EEPROM.h>
#define EEPROM_SIZE 512
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
#include <WiFiUdp.h>
const long utcOffsetInSeconds = 19800; //+5:30 For correct time in India
int currentHour;                       // Current Time in Hour
int currentMinute;                     // Current Time in Minute
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
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

//!========================== VARIABLE DEFINITION ==========================//
int StartHour;                       // Variable to define the Light Start Time
int StopHour;                        // Variable to define the Light Stop Time
int StartMinute;                     // Variable to define the Light Start Time
int StopMinute;                      // Variable to define the Light Stop Time
bool lightStatus = LOW;              // Current Status of Light Relay
uint32_t start_time;                 // Time in minutes: (StartHour*60) + StartMinutes
uint32_t stop_time;                  // Time in minutes: (StopHour*60) + StopMinutes
uint32_t current_time;               // Time in minutes: (currentHour*60) + currentMinutes
uint32_t time_at_twelve = 720;       // Mins
uint32_t time_at_twenty_four = 1440; // Mins
int interrupt_status = 0;            // 1 = Interrupt Detected, 0 = No Interrupt
//!===========================================================================//

//!========================== FUNCTION DEFINITION ==========================//
IRAM_ATTR void change_settings(void);
void connect_to_wifi_and_fetch_time();
void turn_on_delay_time(int state);
void turn_off_delay_time(int state);
void turnOffLight(int state);
void turnOnLight(int state);
void make_relay_trigger_decision();
void notFound(AsyncWebServerRequest *request);
void change_configuration_settings(void);
void basic_setup(void);
//!===========================================================================//

//!========================== FUNCTION DECLARATION ==========================//
/**
 * @brief This function changes the status of interrupt_status which is used to call the change_configuration_settings function
 * @return IRAM_ATTR
 */
IRAM_ATTR void change_settings(void)
{
  delay(100);
  interrupt_status = 1;
}

/**
 * @brief Conencts to wifi and fetches the latest time from ntp server
 */
void connect_to_wifi_and_fetch_time()
{
  // Connecting to Wi-Fi
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(50);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected to: " + String(ssid));

  // Fetching Time from NTP
  timeClient.begin();
  for (size_t i = 0; i < 3; i++) // Fetching it thrice to avoid errors
    timeClient.update();
  Serial.println("Time: " + String(timeClient.getFormattedTime()));
  currentHour = timeClient.getHours();     // Storing Current Time into Variable
  currentMinute = timeClient.getMinutes(); // Storing Current Time into Variable
  current_time = (currentHour * 60) + currentMinute;
  Serial.println("Current Time (mins): " + (String)current_time);
  make_relay_trigger_decision();
  return;
}

/**
 * @brief Calculates the ON Delay Timer
 * @param state (0 => Stop hour after Midnight),(1 => Stop Hour before Midnight)
 */
void turn_on_delay_time(int state)
{
  Serial.println("Turn ON Delay Timer Starting");
  uint32_t delay_in_minutes = 0;
  if (state == 0)
    delay_in_minutes = stop_time - current_time;
  else if (state == 1)
    if (current_time > time_at_twelve)
      delay_in_minutes = (time_at_twenty_four - current_time) + stop_time;
    else
      delay_in_minutes = stop_time - current_time;

  delay_in_minutes = delay_in_minutes / 2; // Sleeping only for half the time
  Serial.println("Timer Duration: " + (String)delay_in_minutes + " mins");
  for (size_t i = 0; i < (delay_in_minutes * 600); i++)
  {
    delay(100);
    if (interrupt_status == 1)
      break;
  }
  Serial.println("Turn ON Delay Timer Ended");
  return;
}

/**
 * @brief Calculates the OFF Sleep Timer
 * @param state (0 => Stop hour after Midnight),(1 => Stop Hour before Midnight)
 */
void turn_off_delay_time(int state)
{
  Serial.println("Sleep Timer Starting");
  uint32_t sleep_duration_in_seconds = 0;
  if (state == 1)
    sleep_duration_in_seconds = (start_time - current_time) * 60;
  else if (state == 0)
    if (current_time < time_at_twenty_four)
      sleep_duration_in_seconds = ((time_at_twenty_four - current_time) + start_time) * 60;
    else
      sleep_duration_in_seconds = (start_time - current_time) * 60;

  sleep_duration_in_seconds = sleep_duration_in_seconds / 2; // Sleeping only for half the time
  Serial.println("Timer Duration: " + (String)sleep_duration_in_seconds + " Seconds");
  ESP.deepSleep(sleep_duration_in_seconds * 1000000);
  Serial.println("Just Woke Up");
}

/**
 * @brief This function turns Off the light relay and then calls the sleep timer
 * @param state (0 => Stop hour after Midnight),(1 => Stop Hour before Midnight)
 */
void turnOffLight(int state)
{
  digitalWrite(LIGHT_RELAY, LOW);
  lightStatus = 0;
  Serial.println("Light OFF");
  turn_off_delay_time(state);
  return;
}

/**
 * @brief This function turns on the light relay and then calls the delay timer
 * @param state (0 => Stop hour after Midnight),(1 => Stop Hour before Midnight)
 */
void turnOnLight(int state)
{
  digitalWrite(LIGHT_RELAY, HIGH);
  lightStatus = 1;
  Serial.println("Light ON");
  turn_on_delay_time(state);
  return;
}

/**
 * @brief Code for Decision Making to decide whether to turn On/Off Light
 */
void make_relay_trigger_decision()
{
  Serial.println("Making Decision for the Light");
  if (StartHour > StopHour) // If the Set Stop Time is after midnight
  {
    Serial.println("Stop Time after Midnight");
    if ((current_time > start_time) || (current_time < stop_time))
      turnOnLight(0);
    else
      turnOffLight(0);
  }
  else // Condition where the Set Stop hour is before MidNight
  {
    Serial.println("Stop Time before Midnight");
    if ((current_time > start_time) && (current_time < stop_time))
      turnOnLight(1);
    else
      turnOnLight(1);
  }
}

/**
 * @brief Function is used when there is error while creating Web Server
 * @param request
 */
void notFound(AsyncWebServerRequest *request)
{
  Serial.println("Cannot Create Server");
  request->send(404, "text/plain", "Not found");
}

/**
 * @brief Creates a Web Server which is used to enter new configuration settings like:
 * SSID
 * Password
 * Start Hour
 * Start Minutes
 * Stop Hour
 * Stop Minutes
 */
void change_configuration_settings(void)
{
  Serial.println("Some Configurations are missing.\nSetting up Configuration Page");
  WiFi.disconnect();
  digitalWrite(LED_PIN, HIGH);

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

/**
 * @brief Read the configuration details from eeprom
 */
void read_configuration_details_from_eeprom(void)
{
  Serial.println("Reading Configuration Files");

  EEPROM.begin(EEPROM_SIZE);
  // TODO: Uncomment these lines when EEPROM needs to be cleared
  //  for (size_t i = 0; i < EEPROM_SIZE; i++)
  //    EEPROM.write(i, 0);
  //  EEPROM.commit();

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
  start_time = (StartHour * 60) + StartMinute;
  Serial.println("Start Time (mins): " + (String)start_time);

  StopHour = EEPROM.read(SAVED_STOP_HOUR_LOCATION);
  StopMinute = EEPROM.read(SAVED_STOP_MINUTES_LOCATION);
  Serial.println("Stop Time: " + (String)StopHour + ":" + (String)StopMinute);
  stop_time = (StopHour * 60) + StopMinute;
  Serial.println("Stop Time (mins): " + (String)stop_time);
  // checking if all parameters are correct or not
  if ((ssid_length == 0) || (password_length == 0) || (StartHour == StopHour))
    change_configuration_settings();
}

/**
 * @brief Configure GPIO Pins and attach the interrupts and make the basic setup
 */
void basic_setup(void)
{
  Serial.println("\n\n\n\nBasic Setup");
  pinMode(LIGHT_RELAY, OUTPUT);                                                // Setting Relay Pin to Output
  pinMode(LED_PIN, OUTPUT);                                                    // LED Button to indicate whether you are in configuration setting or not
  attachInterrupt(digitalPinToInterrupt(INT_BUTTON), change_settings, RISING); // Interrupt button to change configuration settings
  digitalWrite(LIGHT_RELAY, LOW);
  digitalWrite(LED_PIN, LOW);
  read_configuration_details_from_eeprom();
}

//!===========================================================================//

void setup()
{
  Serial.begin(115200); // Enabling Serial Port
  basic_setup();
  Serial.println("ESP8266 Loop Start");
  connect_to_wifi_and_fetch_time();
}

void loop()
{
  if (interrupt_status == 1)
  {
    interrupt_status = 0;
    change_configuration_settings();
  }
  connect_to_wifi_and_fetch_time();
}