# Welcome to the *fresca* project!  

![Build: passing](https://img.shields.io/badge/build-passing-green.svg)
[ ![Licence: GPL-3.0](https://img.shields.io/badge/licence-GPL--3.0-blue.svg) ](https://www.gnu.org/licenses/gpl-3.0)


This project aims to develop a multi-sensor (up to 8) temperature process control platform with configurable heating/cooling actuators and remote host access/monitoring.  
The process controller is called **fresca-controller** and is based on an Arduino platform and the DS18B20 and/or DHT22 digital sensors. It supports setting temperature operating point with hystheresis 
via an LCD Keypad shield, and supports temperature display with additional 7-segment displays for each sensor. The controller will turn on/off the heating/cooling actuators according to configurable
temperature thresholds for each sensor.  
The host handles connection to the controller using **fresca-link** (UART) and provides remote access to the controller via **fresca-webapp**.
It is usually run on a single board computer like a Raspberry Pi.

## Features
***

**fresca-controller**:

* +-0.5deg celsius accuracy from -10deg to +85deg without calibration (Higher accuracy is possible through offset calibration) (DS18B20)
* Relative humidity sensing with DHT22 sensor
* Interactive menu to modify sensor/temperature control parameters
* Monitoring of temperature for each sensor on 7-segment displays (Celsius/Fahrenheit)
* CoolOn/CoolOff and HeatOn/HeatOff thresholds offer hysteresis-like temperature control for each sensor
* Offset calibration for each sensor, stored in the DS18B20's EEPROM (each sensor will store the calibration data)
* Storage of project settings in Arduino's EEPROM
* Sensor CRC and presence checking (no wrong read-outs)

**fresca-link**:  

* Data-gathering from controller via UART-link
* Logging of controller variables to disk (Time/Temperature/Humidity/Controller state)

**fresca-webapp**:  

* Remote monitoring via webapp (python/flask based using Google Charts)

## Parts

**fresca-controller** parts:  

* *Arduino Mega 2560* or similar (depending on how many processes are needed)  
* *DFR Robot LCD keypad shield* for user interface/control (one)  
* *DS18B20* digital temperature sensor with OneWire interface (as many as processes are)  
* *DHT22*  digital temperature/humidity sensor (as many as processes are)  
* 7-segment displays based on TM1637 controller (as many as processes are)  
* 5v Relays as actuators (as many as processes are)  

**fresca-link** and **fresca-webapp** parts:  

* Host computer with internet connection (Raspberry Pi, PC or other)  
* A **fresca-controller**  

## NOTES
***
  
* **PLEASE NOTE:** This repo contains submodules, so for the first time cloning the repo you can do it like 
this: `git clone --recursive https://github.com/lcapossio/fresca.git` , or if you already cloned 
you have to initialize the submodules: `git submodule update --init --recursive`. Also each time you pull, 
you have to pull recursively, in case there is an update for the submodules: `git pull --recurse-submodules`  
* Although this project was first started to aid in beer fermentation, it can be used in any temperature control 
project that has similar specifications
* Source code is released under GPLv3, please read LICENSE for more information

*PD: The project is called 'fresca' in reference to Argentinian slang word used to describe a cold beer! Plus direct-translation of fresca is **cool** :)*