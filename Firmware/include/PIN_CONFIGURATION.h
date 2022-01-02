#ifndef PIN_CONFIGURATION_H
#define PIN_CONFIGURATION_H

//!========================== PIN DEFINITION ==========================//
#define ESP8266_GPIO_16 16 // WAKE
#define ESP8266_GPIO_5 5   // SCL
#define ESP8266_GPIO_4 4   // SDA
#define ESP8266_GPIO_0 0   // FLASH
#define ESP8266_GPIO_2 2   // TXD1
#define ESP8266_GPIO_14 14 // SCLK
#define ESP8266_GPIO_12 12 // MISO
#define ESP8266_GPIO_13 13 // MOSI
#define ESP8266_GPIO_15 15 // CS
#define ESP8266_GPIO_3 3   // RXD0
#define ESP8266_GPIO_1 1   // TXD0
#define ESP8266_GPIO_10 10 // SDD3
#define ESP8266_GPIO_9 9   // SDD2
//!====================================================================//

//!========================== PIN CONNECITON ==========================//
#define LIGHT_RELAY ESP8266_GPIO_4 // Relay Pin to control the led panel for the night mode of the name plate
#define INT_BUTTON ESP8266_GPIO_14 // Interrupt button used to add new wifi in case of any change
#define LED_PIN ESP8266_GPIO_2     // LED Pin to denote when the device is in configuration mode
//!====================================================================//

#endif