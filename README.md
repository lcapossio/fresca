# Welcome to the *fresca* project!  

## BRIEF DESCRIPTION 
***

This project aims to develop a temperature controller for beer fermenters (8 or more). The controller is based on an Arduino and the DS18B20 digital temperature sensor. It supports setting temperature operating point with hystheresis via an LCD Keypad shield, and support temperature display with additional 7-segment displays for each sensor. The controller will open valves (with relays) carrying cold liquid (water) that will run inside the fermenter through a serpentine coil thus cooling the beer.

Project parts:  
* *Arduino Mega 2560* or similar (depending on how many fermenters are needed)  
* LCD keypad shield (1)  
* *DS18B20* digital temperature sensor with OneWire interface (as many as needed)  
* 7-segment displays (as many as needed)  
* 5v Relays (as many as needed)  

All the parts are readily-available arduino parts, so there should be no trouble getting them.  

*NOTE: Although this project was first started to aid in beer fermentation, it can be used in any temperature control project that has similar specifications.*

## Features
***
The current code has the following features:

* +-0.5deg celsius accuracy from -10deg to +85deg without calibration (Higher accuracy is possible through calibration)
* Interactive menu to modify sensor/temperature control parameters
* Displaying of temperature for each sensor on 7-segment displays, regardless of user input
* CoolOn/CoolOff thresholds offer hysteresis-like temperature control for each sensor
* Offset calibration for each sensor, stored in the DS1820's EEPROM (sensor will store the calibration data)
* Storage of settings in Arduino's EEPROM
* Sensor CRC and presence checking (no wrong read-outs)
* Temperature display in celsius/fahrenheit

## Usage of fresca
***

The repo contains two programs, they reside under the *'src'* folder.  

* **'test'** is designed to test the basic parts of the project: only one sensor, one relay and one 7-segment displays are supported
* **'fresca'** is the main program, it currently supports up to 8 sensors/7-segment/relays

### Main menu

All user input/interaction is displayed on the LCD, and user input is possible by using the keypad.
The 7-segment displays monitor the temperature of each sensor. This is regardless of what mode the program is in.
Temperature updates every second (actually a bit faster, 900ms)

### Menu navigation
On the main screen temperature is displayed for the current selected sensor (default sensor 0). Using the *UP/DOWN* arrows selects a different sensor.

While on the main screen, if the *SEL* key is pressed the program will enter configuration mode for the given sensor. On the first configuration screen the *'CoolOn'* threshold can be modified. Use *LEFT/RIGHT* arrows to change the temperature above which the relay for cooling will be activated. Then press *SEL*. The next screen modifies the *'CoolOff'* threshold, also set it with *LEFT/RIGHT* arrows. If the temperature falls below this threshold the MCU will deactivate the respective relay. The next screen is accessed by pressing *SEL* again. This screen allows to modify the offset of the temperature reading of the sensor. This gives the possibility to calibrate the sensor to a known reference temperature. By pressing *SEL* once more, all settings are saved and the program returns to the main screen.

### Pinout
The pinout for the code can be modified easily in **'fresca.ino'**, look for the line that says:
`
// ****** DEFINE PINOUT HERE ****** PINOUT_LINE
`

### Celsius/Fahrenheit

To display temperature in fahrenheit set the **TEMP_FAHRENHEIT** constant in **'fresca.h'** to other value than zero. Otherwise temperature is displayed in degrees celsius.

### Further customization/debugging

Many constants that define program behavior and debugging are defined in *'fresca.h'*, they are commented so you can easily find them and edit them!

### Resource usage

**'fresca.ino'** uses on an Arduino Mega 2560 :
> Sketch uses 10894 bytes (4%) of program storage space. Maximum is 253952 bytes.
> Global variables use 990 bytes (12%) of dynamic memory, leaving 7202 bytes for local variables. Maximum is 8192 bytes.

Free memory in runtime is around 6900 bytes

## Further improvements planned
***

* Add heating to the temperature control
* Add Web server to monitor temperature using a wifi shield

## NOTES
***

_*** Check the WIKI for more info ***_

Source code is released under GPLv3, please read LICENSE for more information

*PD: The project is called 'fresca' in reference to Argentinian slang word used to describe a cold beer! :)*