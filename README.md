
# Pulsar 

This repository contains the source code for the P2I-6 project at INSA Lyon. The project's objective is to develop an automated system for launching and monitoring a water rocket from inception.

The project entails constructing both a water rocket and a launch pad. The water rocket will be equipped with a microcontroller and sensors to monitor its flight, while the launch pad will incorporate a microcontroller to control the launch.

Furthermore, the launch pad must be activatable via an Android application. This application will not only display the rocket's flight data but also facilitate launching the rocket at a predetermined pressure. All collected data will be recorded and stored on a dedicated server.

Ultimately, the project aims to experimentally determine the relationship between initial pressure and the rocket's altitude, generalize the results, and predict the altitude achieved from a given initial pressure.


## Boards 

- Launchpad-launcher: the code for the Arduino Uno board that controls the launch pad and the launch of the rocket.
- Launchpad-server: the code for esp32-S2 that serves as a way to communicate with backend.
- Launchpad-rocket: the code for the Feather board that is linked to the rocket to receive flight data and send data to the launchpad-server board.
- Rocket: The code of that is used inside the rocket

## Board instruction 

### Feather board

1. Install platformio plugin in VSCode
2. On linux: https://docs.platformio.org/en/latest/core/installation/udev-rules.html 
