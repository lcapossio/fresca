# Welcome to the *fresca* project!  

This project aims to develop a temperature controller for beer fermenters (8 or more). The controller is based on an Arduino and the DS18B20 digital temperature sensor.
 It supports setting temperature operating point with hystheresis via an LCD Keypad shield, and support temperature display with additional 7-segment displays for each sensor. The controller will open valves (with relays) carrying cold liquid (water) that will run inside the fermenter through a serpentine coil thus cooling the beer.
 The controller also has control for a heating element in case of places with ver low ambient temperature.

Project parts:  

* *Arduino Mega 2560* or similar (depending on how many fermenters are needed)  
* LCD keypad shield (1)  
* *DS18B20* digital temperature sensor with OneWire interface (as many as needed)  
* *DHT22*  digital temperature/humidity sensor (as many as needed)  
* 7-segment displays (as many as needed)  
* 5v Relays (as many as needed)  

All the parts are readily-available arduino parts, so there should be no trouble getting them.  

*NOTE: Although this project was first started to aid in beer fermentation, it can be used in any temperature control project that has similar specifications.*

## Features
***
The current code has the following features:

* +-0.5deg celsius accuracy from -10deg to +85deg without calibration (Higher accuracy is possible through calibration) (DS18B20)
* Relative humidity sensing with DHT22 sensor
* Interactive menu to modify sensor/temperature control parameters
* Monitoring of temperature for each sensor on 7-segment displays, regardless of user input
* CoolOn/CoolOff and HeatOn/HeatOff thresholds offer hysteresis-like temperature control for each sensor
* Offset calibration for each sensor, stored in the DS18B20's EEPROM (each sensor will store the calibration data)
* Storage of settings in Arduino's EEPROM
* Sensor CRC and presence checking (no wrong read-outs)
* Temperature display in celsius/fahrenheit

## Build instructions
***

The project build is now automated by using either **Arduino-Makefile** or **platformio** (your choice), you can still use **Arduino IDE** as well.
The default build is for Arduino Mega2560, but this can be ported to other boards.  

**'PLEASE NOTE:'** This repo contains submodules, so for the first time cloning the repo you can do it like this: **'git clone --recursive https://github.com/lcapossio/fresca.git'**, or if you already cloned 
you have to initialize the submodules: **'git submodule update --init --recursive'**. Also each time you pull, you have to pull recursively, in case there is an update for the submodules: **'git pull --recurse-submodules'**  

Using **Arduino IDE**:  

* Add all the libraries under **'arduino/lib'**
* The main sketch is **'arduino/src/fresca.ino'**
* Compile and upload the sketch

Using **Arduino-Makefile**:  

* **'cd'** to the **'arduino/src'** folder  
* run **'make ARDUINO_DIR=your/arduino/install/path'**  

Using **platformio**:  

* **'cd'** to the **'arduino'** folder  
* run **'platformio run'**  

Check the **'Configuration/Pinout'** section for info on customizing **'fresca'**

## Usage of fresca/UI description
***

The main sketch resides under the *'arduino/src'* folder. All the libraries are under *'arduino/lib'*

* **'fresca'** is the main program, it currently supports up to 8 sensors/7-segment/relays

### Main menu

All user input/interaction is displayed on the LCD, and user input is possible by using the keypad.
The 7-segment displays monitor the temperature of each sensor. This is regardless of what mode the program is in.
Temperature updates every second (actually a bit faster, around 900ms)

### Menu navigation
On the main screen temperature is displayed for the current selected sensor (default sensor 0). Using the *UP/DOWN* arrows selects a different sensor. Pressing *LEFT/RIGHT* buttons will toggle temperature/humidity display (for sensors that support it)

While on the main screen, if the *SEL* key is pressed the program will enter configuration mode for the given sensor. The first configuration screen the *'CoolOn'* threshold can be modified. Use *LEFT/RIGHT* arrows to change the temperature above which the relay for cooling will be activated. Then press *SEL*. The next screen modifies the *'CoolOff'* threshold, also set it with *LEFT/RIGHT* arrows. If the temperature falls below this threshold the MCU will deactivate the respective relay. The next screen is accessed by pressing *SEL* again.

The next two screens are dedicated to the heating part of the controller. *'HeatOn'* will turn on the heating relay if temperature falls below this threshold. When you are done press *SEL* to continue. *'HeatOff'* will turn off the heating relay when temperature rises above said threshold. Press *SEL* after this to continue to the offset calibration.

This screen allows to modify the offset of the temperature reading of the sensor. This gives the possibility to calibrate the sensor to a known reference temperature. By pressing *SEL* once more, all settings are saved and the program returns to the main screen.

NOTE: If heating and cooling parts of the controller overlap, cooling will take precedence.

### Configuration/Pinout
The configuration/pinout for the code can be modified easily in **'arduino/lib/fresca/fresca_pinout.h'**. Look at the commented description of each line to know what they are used for.

Keypad is connected to an analog pin.
Each sensor is connected to a single digital I/O.
7-Segment displays CLK pins are connected to a single arduino digital pin.

### Celsius/Fahrenheit

To display temperature in fahrenheit set the **TEMP_FAHRENHEIT** define in **'fresca.h'** to other value than zero. Otherwise temperature is displayed in degrees celsius.

### Further customization/debugging

Many constants that define program behavior and debugging are defined in *'fresca.h'*, they are commented so you can easily find them and edit them!

### Resource usage

**'fresca.ino'** uses on an Arduino Mega 2560 for 8 DS18B20 sensors:
> Program memory: 15536 bytes (5.9% Full)  
> Data Memory: 1162 bytes (14.2% Full)  

Free memory in runtime is around 6900 bytes

### Libraries

This project uses three libraries which can be found in the *'lib'* directory. These libraries have been modified slightly and are property of their respective owners
The third-party libraries used are: 
* DFR_Key (for analog keypad)
* OneWire (for DS1820)
* TM1637-1.1.0_7seg (for 7-segment display controller)
* DHT from Adafruit industries (for DHT22)

Grab them and install them in your Arduino IDE (you don't need to do this if you are building from platformio or Arduino-Makefile)

## Further improvements planned
***

* Support for other keypads
* Add web server and data logging with a Raspberry Pi (via SPI)

## Why only one sensor per wire ?

As many of you know it is possible to accommodate many OneWire sensors in a single wire. Even though this represents a significant reduction in pin count, it makes things more complicated for the user. With one sensor per wire, the user can clearly identify what thresholds temperature reading belong to which sensor, and can even interchange sensors and replace them freely. Otherwise there would need to be a ROM matching mechanism for new/current sensors to respective thresholds. So the setup is easier and there is less confusion, and getting a high pin-count arduino is really cheap.

## NOTES
***

_*** Check the WIKI for more info ***_

Source code is released under GPLv3, please read LICENSE for more information

*PD: The project is called 'fresca' in reference to Argentinian slang word used to describe a cold beer! :)*