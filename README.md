# German Smartknob
An ESP32-S3 based knob to control smart devices at home.

## Table of Contents
1. [About the Project](#about-the-project)
2. [Features](#features)
3. [Components](#components)
4. [Getting Started](#getting-started)
5. [Project Status](#project-status)

## About the project
Project i developed for my Bachelor Thesis. Based on Scott Bez's Smartknob: https://github.com/scottbez1/smartknob?tab=readme-ov-file. Adapted for the standard german wall socket.
<p align="center">
<img src="images/exploded_view.png" alt="explodedview" width="500" height="700" style="display:inline-block;" />
</p>

## Features
- ESP32-S3 based
- Input Voltage of up to 24V
- Haptic Feedback with a BLDC-Motor
- Touch Display
- Brightness and Temperature Sensor
- Audio speaker connectors

## Components
| Component | Model  | Manufacturer  |
|:--------|:------:|------:|
| Microcontroller  | atmega168 | Microchip |
| 7 Seg. Display  | Generic | - |
| Rotary Encoder Module  | KY-040 | - |
| Battery Charger  | BQ24090 | Texas Instruments |

## Getting Started

### Download this repo
```
git clone https://github.com/LucasCanete/GermanSmartknob.git
```
### Build and upload 


### How To Use
The Kitchenalarm has a **Rotary Encoder** that lets you configure the Minutes and Seconds through the encoder's rotation.
1. Press the Rotary Encoder on the board to configure the Minutes and adjust it rotating the encoder 
2. Press it once again to configure the Seconds and rotate it to adjust 
3. Press the encoder one last time to activate the Timer with the Minutes and Seconds you configured in step 1 and step 2. The Timer will start counting down on the 7-Segment Display (To reset the countdown press the Encoder)
4. When the Timer reaches 00:00 (on the Display) the buzzer on the Board will start beeping. Your pizza is ready. Press the Encoder once to stop the beeping
5. The Kitchenalarm goes automatically to deep sleep if it is not given instructions. To wake it up just press the Encoder


<p align="center">
   <img src="images/pizzaclock_pcb.jpeg" alt="Design" width="300" height="320" style="display:inline-block; margin-right: 10px;" /> 
   <img src="images/pizzaclock_case.jpeg" alt="kitchenalarm" width="300" height="350" style="display:inline-block;" />
</p>

## Project Status
Coming soon
