# Automatic Name & Address Plate for House

## What the Project does?

I always wanted a glowing address plate for my house. Buy alas, I was born lazy !! I hate to turn ON and OFF the lights of my address plate for my house everyday. So, I have created an automatic address plate which turns the backlight ON/OFF on the programmed time. The base microcontroller used is [Node MCU](https://www.amazon.in/gp/product/B010O1G1ES/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) which connects to my home Wifi. After connecting to Wifi, it contacts the NTP server and updates it current time.

### Sleep Duration

- If current time is less than Backlight start time, it sleeps for half the remaining time. After waking up, it does the same process until current time equals set start time.

### Turn On Duration

- If current time is greater than Backlight start time but less than Backlight stop time, it creates a delay for half the remaining time. This process continues till stop time is reached.

## Components Used

- NodeMCU ESP8266 - [Amazon Link](https://www.amazon.in/gp/product/B010O1G1ES/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)

<br />
<br />

### Tools Used:

<img width="40px" align="left" alt="ESP8266" src="Resources/Logos/ESP8266.png">
<img width="50px" align="left" alt="MATLAB" src="Resources/Logos/Arduino Logo.png">
<img width="30px" align="left" alt="C" src="Resources/Logos/C Logo.jpeg">
<img width="25px" align="left" alt="CPP" src="Resources/Logos/CPP Logo.jpeg">
<img width="40px" align="left" alt="Embedded C" src="Resources/Logos/Embedded C Logo.jpeg">
<img width="80px" align="left" alt="Git" src="Resources/Logos/Git Logo.jpeg">
<img width="35px" align="left" alt="GitHub" src="Resources/Logos/GitHub Logo.jpeg">
<img width="35px" align="left" alt="VS Code" src="Resources/Logos/VS Code.jpeg">
<img width="30px" align="left" alt="Eagle" src="Resources/Logos/Eagle Logo.png">
<img width="35px" align="left" alt="Ideamaker" src="Resources/Logos/Ideamaker Logo.png">
<img width="22px" align="left" alt="Fusion 360" src="Resources/Logos/Fusion 360 Logo.png">
