This repository contains ESP IDF project files of project 'Visitor Control System' or, alternatively, 'VCS'. This project was fulfilled by Lviv Polytechnic National University studentds in order to improve microcontroller programming, team working, client-server workflow and other skills.

The main idea was to create system that performs optionally one of two functions:
  - tracking visitors of any facility to estimate their total number and activity
  - perform alarm function when facility is closed.

As main microcontroller we chose ESP32 because:
  - it's popular and verified solution when WiFi is used
  - it's cheap.

To track person entering we used HC-SR04 ultrasonic sensor. There were several reasons to choose this type of sensor:
  - it already was at hand
  - it's cheap
  - it's effective if placed in entrance as it was intended
  - there are corresponding libraries.

To record date and time of person entering we used SNTP and set up WiFi previously. Also, TCP-server was launched on ESP32 board through websocket. Server sent data to mobile app. In that way remote control through the mobile device like phone was established.
