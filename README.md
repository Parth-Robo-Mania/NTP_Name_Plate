# Automatic Name & Address Plate for House

## What the Project does?

I always wanted a glowing address plate for my house. Buy alas, I was born lazy !! I hate to turn ON and OFF the lights of my address plate for my house everyday. So, I have created an automatic address plate which turns the backlight ON/OFF on the programmed time. The base microcontroller used is [Node MCU](https://www.amazon.in/gp/product/B010O1G1ES/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) which connects to my home Wifi. After connecting to Wifi, it contacts the NTP server and updates it current time.

### Sleep Duration

- If current time is less than Backlight start time, it sleeps for half the remaining time. After waking up, it does the same process until current time equals set start time.

### Turn On Duration

- If current time is greater than Backlight start time but less than Backlight stop time, it creates a delay for half the remaining time. This process continues till stop time is reached.

## Components Used

- NodeMCU ESP8266 - [Amazon Link](https://www.amazon.in/gp/product/B010O1G1ES/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
