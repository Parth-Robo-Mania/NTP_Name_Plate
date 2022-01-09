/**
 * @file PIN_CONFIGURATION.h
 * @author Parth Patel (parth.pmech@gmail.com)
 * @brief This file contains pin configuration for the board
 * @version 0.1
 * @date 2022-01-02
 * @copyright Copyright (c) 2022
 */
#ifndef PIN_CONFIGURATION_H
#define PIN_CONFIGURATION_H

/* -------------------------------------------------------------------------- */
/*                               PIN DEFINITION                               */
/* -------------------------------------------------------------------------- */
// GPIO Pins
#define ESP32_GPIO_2 2
#define ESP32_GPIO_4 4 // Interrupt button used to add new wifi in case of any change
#define ESP32_GPIO_5 5
#define ESP32_GPIO_12 12
#define ESP32_GPIO_13 13
#define ESP32_GPIO_14 14
#define ESP32_GPIO_15 15
#define ESP32_GPIO_16 16
#define ESP32_GPIO_17 17
#define ESP32_GPIO_18 18
#define ESP32_GPIO_19 19
#define ESP32_GPIO_21 21 // LED Pin to denote when the device is in configuration mode
#define ESP32_GPIO_22 22 // Relay Pin to control the led panel for the night mode of the name plate
#define ESP32_GPIO_23 23
#define ESP32_GPIO_25 25
#define ESP32_GPIO_26 26
#define ESP32_GPIO_27 27
#define ESP32_GPIO_32 32
#define ESP32_GPIO_33 33
#define ESP32_GPIO_34 34
#define ESP32_GPIO_35 35
#define ESP32_GPIO_39 39
#define ESP32_GPIO_36 36

/* -------------------------------------------------------------------------- */
/*                               PIN CONNECITON                               */
/* -------------------------------------------------------------------------- */
#define LIGHT_RELAY ESP32_GPIO_22 // Relay Pin to control the led panel for the night mode of the name plate
#define INT_BUTTON ESP32_GPIO_4   // Using this Pin because it supports Interrupt from Deep Sleep
#define LED_PIN ESP32_GPIO_21     // LED Pin to denote when the device is in configuration mode

#endif