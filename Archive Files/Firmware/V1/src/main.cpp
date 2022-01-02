#include <Arduino.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//Relay Pin Connection at D4

//Variable Definition
const uint8_t setStartHour = 18U;                     //Variable to define the Light Start Time of 6:00 PM
const uint8_t setStopHour = 9U;                       //Variable to define the Light Stop Time of 6:00 AM
const uint8_t setStartMinute = 0U;                    //Variable to define the Light Start Time of 6:00 PM
const uint8_t setStopMinute = 59U;                    //Variable to define the Light Stop Time of 6:59 AM
const uint8_t setStartSecond = 0U;                    //Variable to define the Light Start Time of 6:00:00 PM
const uint8_t setStopSecond = 59U;                    //Variable to define the Light Stop Time of 6:59:59 AM
uint8_t currentHour;                                  //Current Time in Hour
uint8_t currentMinute;                                //Current Time in Minute
uint8_t currentSecond;                                //Current Time in Second
uint8_t currentHour1;                                 //Current Time in Hour for verification
const float hourToMinuteConversion = 60U;             // 1hr * 60min
const float minuteToSecondConversion = 60U;           // 1min * 60sec
const float secondToMicroSecondConversion = 1000000U; // 1sec * 1000000uSec
const float secondToMillisecondConversion = 1000U;    // 1sec * 1000ms
bool lightStatus = LOW;                               //Current Status of Light Relay
float turnOnDelay;                                    // Duration to keep Lamp on before rechecking Time
float sleepDuration;                                  //Variable to define the sleep duration

//Wifi Parameters
const char *ssid = "SSD";
const char *password = "1@LucyMorganSmith";

//Time Correction Parameters
const long utcOffsetInSeconds = 19800; //+5:30 For correct time in India

//Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//Connecting to Wifi
void connectToWifi()
{
retryTime:
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to: " + String(ssid));
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
  currentHour = timeClient.getHours();     //Storing Current Time into Variable
  currentMinute = timeClient.getMinutes(); //Storing Current Time into Variable
  currentSecond = timeClient.getSeconds(); //Storing Current Time into Variable
  return;
}

// Code to decide how much delay is needed to keep the light ON
void turnOnDelayDecision()
{
  Serial.println("Turn On Delay Decision");
  if (setStartHour > setStopHour)
  { //Code will execute only when the Timer ends after midnight
    Serial.println("Timer Ends after midnight");
    if ((currentHour < 24) && (currentHour >= setStartHour))
    {
      Serial.println("Timer already started and its before midnight");
      turnOnDelay = ((((24 - currentHour - 1) + setStopHour) * hourToMinuteConversion) + (60 - currentMinute + setStopMinute)) * minuteToSecondConversion / 2;
    }
    else if ((currentHour >= 0) && (currentHour <= setStopHour))
    {
      if (currentHour < setStopHour)
      {
        Serial.println("Timer already started and its after midnight");
        turnOnDelay = ((setStopHour - currentHour) * hourToMinuteConversion * minuteToSecondConversion) / 2;
      }
      else if (currentHour == setStopHour)
      {
        if (currentMinute < setStopMinute)
        {
          Serial.println("Timer already started and its after midnight and about to end in minutes");
          turnOnDelay = (setStopMinute - currentMinute) * minuteToSecondConversion / 2;
        }
        else if (currentMinute == setStopMinute)
        {
          Serial.println("Timer already started and its after midnight and about to end in seconds");
          turnOnDelay = (setStopSecond - currentSecond) / 2;
        }
      }
    }
  }
  else
  { //Code will execute only when the Timer ends before Midnight
    Serial.println("Timer Ends before Midnight");
    turnOnDelay = ((((setStopHour - (currentHour - 1)) * hourToMinuteConversion) + ((60 - currentMinute) + setStopMinute))) * minuteToSecondConversion / 2;
  }
  Serial.println("Setting Light ON Delay for: " + String(turnOnDelay) + " Seconds or " + String(turnOnDelay / (minuteToSecondConversion)) + " Minutes or " + String(turnOnDelay / (minuteToSecondConversion * hourToMinuteConversion)) + " Hour");
  int currentmillis = millis();
  while (millis() - currentmillis < (turnOnDelay * secondToMillisecondConversion))
  {
    delay(1000);
  }
  Serial.println("Delay of ON Light Ended");
  return;
}

//Code to Decide the Deep Sleep Timer
void turnOffDelayDecision()
{
  Serial.println("Turn Off Delay Decision");
  if (setStartHour > setStopHour)
  { // If Stop Hour is after MidNight
    Serial.println("Timer Ends after midnight");
    if (setStartHour - currentHour > 2)
    {
      Serial.println("Timer gao still more than 2 hours");
      sleepDuration = 1 * hourToMinuteConversion * minuteToSecondConversion;
    }
    else if (setStartHour - currentHour > 1)
    {
      Serial.println("Timer gao more than 1 hour");
      sleepDuration = 1 * hourToMinuteConversion * minuteToSecondConversion / 2;
    }
    else if (currentMinute < 59)
    {
      Serial.println("Timer reached " + String(setStartHour) + " and the delay shall be in minutes now");
      sleepDuration = (60 - currentMinute) * minuteToSecondConversion / 2;
    }
    else
    {
      Serial.println("Timer reached " + String(setStartHour) + " and the delay shall be in seconds now");
      sleepDuration = (60 - currentSecond) / 2;
    }
  }
  else
  {
    Serial.println("Timer ends before midnight");
    if (currentHour < setStartHour)
    {
      Serial.println("Still waiting for timer to start");
      sleepDuration = ((60 - currentMinute) + ((setStartHour + (currentHour - 1)) * hourToMinuteConversion)) * minuteToSecondConversion / 2;
    }
    else if (currentHour == setStartHour)
    {
      Serial.println("Timer reached start Hour");
      if (currentMinute < 59)
      {
        Serial.println("Timer starting in seconds");
        sleepDuration = (60 - currentSecond) / 2;
      }
      else
      {
        Serial.println("Timer starting in minutes");
        sleepDuration = (60 - currentMinute) * minuteToSecondConversion / 2;
      }
    }
    else if ((currentHour > setStopHour) && (currentHour < 24))
    {
      Serial.println("Timer already ended");
      sleepDuration = (((24 - currentHour) + setStartHour) * hourToMinuteConversion) * hourToMinuteConversion * minuteToSecondConversion / 2;
    }
  }
  Serial.println("Entering Sleep Duration for: " + String(sleepDuration) + " seconds or " + String(sleepDuration / minuteToSecondConversion) + " Minutes or " + String(sleepDuration / (minuteToSecondConversion * hourToMinuteConversion)) + " Hour");
  ESP.deepSleep(sleepDuration * secondToMicroSecondConversion);
}

//Code to Turn OFF Light
void turnOffLight()
{
  digitalWrite(D4, LOW);
  lightStatus = 0;
  Serial.println("Light Status:" + String(lightStatus));
  turnOffDelayDecision();
  return;
}

//Code to Turn ON Light
void turnOnLight()
{
  digitalWrite(D4, HIGH);
  lightStatus = 1;
  Serial.println("Light Status:" + String(lightStatus));
  turnOnDelayDecision();
  return;
}

//Code for Decision Making to decide whether to turn On/Off Light
void decisionMakingMode()
{
  Serial.println("Decision Making");
  if (setStartHour > setStopHour)
  { //If the Set Stop Time is after midnight
    if (((currentHour >= setStartHour) && (currentHour < 24)) | ((currentHour >= 0) && (currentHour <= setStopHour)))
    {
      turnOnLight();
    }
    else
    {
      turnOffLight();
    }
  }
  else
  { //Condition where the Set Stop hour is before MidNight
    if ((currentHour >= setStartHour) && (currentHour <= setStopHour))
    {
      turnOnLight();
    }
    else
    {
      turnOffLight();
    }
  }
}

void setup()
{
  Serial.begin(115200); //Enabling Serial Port
  pinMode(D4, OUTPUT);  //Setting Relay Pin to Output
  Serial.println("ESP8266 Loop Start");
  connectToWifi();
}

void loop()
{
  decisionMakingMode(); //Decide on whether to turn ON/OFF Lights or to go to sleep
  connectToWifi();
}