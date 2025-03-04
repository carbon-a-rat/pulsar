
# Pulsar 

This repository contains the source code for the P2I-6 project at INSA Lyon. The project's objective is to develop an automated system for launching and monitoring a water rocket from inception.

The project entails constructing both a water rocket and a launch pad. The water rocket will be equipped with a microcontroller and sensors to monitor its flight, while the launch pad will incorporate a microcontroller to control the launch.

Furthermore, the launch pad must be activatable via an Android application. This application will not only display the rocket's flight data but also facilitate launching the rocket at a predetermined pressure. All collected data will be recorded and stored on a dedicated server.

Ultimately, the project aims to experimentally determine the relationship between initial pressure and the rocket's altitude, generalize the results, and predict the altitude achieved from a given initial pressure.


## Boards 

INSA is poor, and they don't have enough ESP32 boards for everyone. So we are using the ESP8266 Feather board from Adafruit.

We will also use an arduino board and another Feather board for the launch pad.

## Board instruction 

### Feather board

follow the tutorial [here](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266/using-arduino-ide).

