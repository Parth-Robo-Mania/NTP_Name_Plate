/* -------------------------------------------------------------------------- */
/*                              PRIMARY INCLUDES                              */
/* -------------------------------------------------------------------------- */
#include <Arduino.h>
#include "PIN_CONFIGURATION.h"
#include "HTML_PAGE.h"
#include "EEPROM_Location.h"
#include "Variable_Declaration.h"

//!========================== FUNCTION DEFINITION ==========================//
IRAM_ATTR void change_settings(void);
void connect_to_wifi_and_fetch_time();
void turn_on_delay_time(bool state);
void turn_off_delay_time(bool state);
void turnOffLight(bool state);
void turnOnLight(bool state);
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
    interrupt_status = true;
    Serial.println("Interrupt Detected");
}

/**
 * @brief Conencts to wifi and fetches the latest time from ntp server
 */
void connect_to_wifi_and_fetch_time()
{
    // Connecting to Wi-Fi
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int reattempts = 0;
    Serial.println("\nConnecting to Wi-Fi");
    while (reattempts < NUMBER_OF_RETRIES) // Tries connecting to Wifi multiple times
    {
        int connection_attempts = 0;
        int max_attempts = MAX_WIFI_CONNECT_ATTEMPT_TIME * 10;
        while (connection_attempts < max_attempts)
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                delay(100);
                Serial.print(".");
                connection_attempts++;
            }
            else
                break;
        }
        reattempts++;
        Serial.println();
        if (WiFi.status() == WL_CONNECTED)
            break;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Wi-Fi Connected.");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // Configure NTP
        delay(500);

        Serial.println("Updating NTP");
        struct tm timeinfo;
        int reattempts = 0;
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("NTP Failed to Update");
            WiFi.disconnect();
            delay(10000);
            ESP.restart();
        }
        // Reading NTP Time
        Serial.println("NTP Update Successful");
        currentHour = timeinfo.tm_hour;  // Storing Current Time into Variable
        currentMinute = timeinfo.tm_min; // Storing Current Time into Variable
        String time = (String)currentHour + ":" + (String)currentMinute;
        Serial.println("Time from NTP: " + time);
        current_time = (currentHour * 60) + currentMinute;
        Serial.println("Current Time (mins): " + (String)current_time);
        make_relay_trigger_decision();
        return;
    }
    else
    {
        Serial.println("\nWiFi Connection Failed");
        change_configuration_settings();
    }
}

/**
 * @brief Calculates the ON Delay Timer
 * @param state (false => Stop hour after Midnight),(true => Stop Hour before Midnight)
 */
void turn_on_delay_time(bool state)
{
    Serial.println("Turn ON Delay Timer Starting");
    uint32_t delay_in_minutes = 0;
    if (state)
        delay_in_minutes = stop_time - current_time;
    else
    {
        if (current_time < stop_time)
            delay_in_minutes = stop_time - current_time;
        else
            delay_in_minutes = TIME_AT_TWENTY_FOUR_IN_MINS - current_time + stop_time;
    }
    delay_in_minutes = delay_in_minutes / 2; // Sleeping only for half the time
    Serial.println("Timer Duration: " + (String)delay_in_minutes + " mins");
    for (size_t i = 0; i < (delay_in_minutes * 600); i++)
    {
        delay(100);
        if (interrupt_status)
            break;
    }
    Serial.println("Turn ON Delay Timer Ended");
    return;
}

/**
 * @brief Calculates the OFF Sleep Timer
 * @param state (false => Stop hour after Midnight),(true => Stop Hour before Midnight)
 */
void turn_off_delay_time(bool state)
{
    Serial.println("Sleep Timer Starting");
    uint32_t sleep_duration = 0;
    if (!state)
        sleep_duration = start_time - current_time;
    else
    {
        if (current_time < TIME_AT_TWENTY_FOUR_IN_MINS)
            sleep_duration = TIME_AT_TWENTY_FOUR_IN_MINS - current_time + start_time;
        else
            sleep_duration = start_time - current_time;
    }
    sleep_duration = sleep_duration / 2; // Sleeping only for half the time
    Serial.println("Timer Duration: " + (String)sleep_duration + " Minutes");
    ESP.deepSleep(sleep_duration * 60 * 1000000);
    Serial.println("Just Woke Up");
}

/**
 * @brief This function turns Off the light relay and then calls the sleep timer
 * @param state (false => Stop hour after Midnight),(true => Stop Hour before Midnight)
 */
void turnOffLight(bool state)
{
    digitalWrite(LIGHT_RELAY, LOW);
    lightStatus = 0;
    Serial.println("Light OFF");
    turn_off_delay_time(state);
    return;
}

/**
 * @brief This function turns on the light relay and then calls the delay timer
 * @param state (false => Stop hour after Midnight),(true => Stop Hour before Midnight)
 */
void turnOnLight(bool state)
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
    bool state = false;
    Serial.println("\nMaking Decision for the Light");
    if (StartHour > StopHour) // If the Set Stop Time is after midnight
    {
        Serial.println("Stop Time after Midnight");
        if ((current_time > start_time) || (current_time < stop_time))
            turnOnLight(state);
        else
            turnOffLight(state);
    }
    else // Condition where the Set Stop hour is before MidNight
    {
        Serial.println("Stop Time before Midnight");
        if ((current_time > start_time) && (current_time < stop_time))
            turnOnLight(!state);
        else
            turnOnLight(!state);
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
    Serial.println("\nDevice Configuration Details Unavailable\nConfiguring WebServer");
    digitalWrite(LIGHT_RELAY, LOW);
    delay(500);

    WiFi.disconnect();
    IPAddress local_ip(192, 168, 1, 1);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.mode(WIFI_AP);
    char temp_ssid[] = "Automatic Name Plate";
    char temp_pwd[] = "123456789";
    WiFi.softAP(temp_ssid, temp_pwd);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);
    Serial.println("Connect to:");
    Serial.println("SSID: " + (String)temp_ssid);
    Serial.println("Password: " + (String)temp_pwd);
    Serial.println("IP address: 192.168.1.1");
    digitalWrite(LED_PIN, HIGH);

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
        EEPROM.write(SAVED_SSID_LOCATION + i, inputMessage[i]);
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
        EEPROM.write(SAVED_PASSWORD_LOCATION + i, inputMessage[i]);
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
        if (digitalRead(INT_BUTTON) == HIGH)
        {
            attachInterrupt(digitalPinToInterrupt(INT_BUTTON), change_settings, RISING); // Interrupt button to change configuration settings
            Serial.println("Shutting down Webserver");
            for (size_t i = 0; i < 10; i++)
            {
                digitalWrite(LED_PIN, HIGH);
                delay(100);
                digitalWrite(LED_PIN, LOW);
                delay(100);
            }
            ESP.restart();
        }
    }
}

/**
 * @brief Read the configuration details from eeprom
 */
void read_configuration_details_from_eeprom(void)
{
    Serial.println("Reading Configuration Files");
    EEPROM.begin(EEPROM_SIZE);

    // Reading ssid and password length from eeprom
    int ssid_length = EEPROM.read(SSID_SIZE_LOCATION);
    int password_length = EEPROM.read(PASSWORD_SIZE_LOCATION);
    Serial.println("SSID Length: " + (String)ssid_length);
    Serial.println("Password Length: " + (String)password_length);

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

    // Reading the configurations from the eeprom
    for (size_t i = 0; i < ssid_length; i++)
        ssid[i] = EEPROM.read(SAVED_SSID_LOCATION + i);
    Serial.println("SSID: " + (String)ssid);
    for (size_t i = 0; i < password_length; i++)
        password[i] = EEPROM.read(SAVED_PASSWORD_LOCATION + i);
    Serial.println("Password: " + (String)password);
}

/**
 * @brief Configure GPIO Pins and attach the interrupts and make the basic setup
 */
void basic_setup(void)
{
    Serial.println("\nBasic Setup");
    pinMode(LIGHT_RELAY, OUTPUT);                                                // Setting Relay Pin to Output
    pinMode(LED_PIN, OUTPUT);                                                    // LED Button to indicate whether you are in configuration setting or not
    attachInterrupt(digitalPinToInterrupt(INT_BUTTON), change_settings, RISING); // Interrupt button to change configuration settings
    digitalWrite(LIGHT_RELAY, LOW);
    digitalWrite(LED_PIN, LOW);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, HIGH); // TODO: Change this pin if you decide to choose any other pin
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason)
    {
    case 2:
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    default:
        Serial.println("Wakeup was not caused by deep sleep");
        break;
    }
    read_configuration_details_from_eeprom();
}

//!===========================================================================//

void setup()
{
    Serial.begin(115200); // Enabling Serial Port
    Serial.println("\n\nSetting Up Device");
    basic_setup();
    Serial.println("\nInitiating Device");
}

void loop()
{
    if (interrupt_status) // When INT is triggered in wake mode
    {
        interrupt_status = false;
        change_configuration_settings();
    }
    else if (wakeup_reason == 2) // When INT is triggered from deep sleep wakeup
        change_configuration_settings();
    connect_to_wifi_and_fetch_time();
}