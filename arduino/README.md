# fresca-controller  

## Parts  

* *Arduino Mega 2560* or similar (depending on how many processes are needed)  
* *DFR Robot LCD keypad shield* for user interface/control (one)  
* *DS18B20* digital temperature sensor with OneWire interface (as many as processes are)  
* *DHT22*  digital temperature/humidity sensor (as many as processes are)  
* 7-segment displays based on TM1637 controller (as many as processes are)  
* 5v Relays as actuators (as many as processes are)  

## Build instructions
***

The project build is now automated by using either **Arduino-Makefile** or **platformio**
(your choice), you can still use **Arduino IDE** as well.
The default build is for Arduino Mega2560, but this can be ported to other boards.  

Using **Arduino IDE**:  

* Add all the libraries under ` /lib `  
* The main sketch is ` /src/fresca.ino `  
* Compile and upload the sketch

Using **Arduino-Makefile**:  

* `cd` to the `/src` folder  
* run `make ARDUINO_DIR=your/arduino/install/path`  

Using **platformio**:  

* run `platformio run`  

Check the **'Configuration/Pinout'** section for info on customizing **'fresca'**

## Usage of fresca/UI description
***

The main sketch resides under the `/src` folder. All the libraries are under `/lib`

* **'fresca'** is the main program, it currently supports up to 8 processes

### Main menu

All user input/interaction is displayed on the LCD, and user input is possible by using the keypad.
The 7-segment displays monitor the temperature of each sensor. This is regardless of what mode the program is in.
Temperature updates every second (actually a bit faster, around 900ms)

### Menu navigation
On the main screen temperature is displayed for the current selected sensor (default sensor 0). Using the *UP/DOWN* arrows selects 
a different sensor. Pressing *LEFT/RIGHT* buttons will toggle temperature/humidity display (for sensors that support it)

While on the main screen, if the *SEL* key is pressed the program will enter configuration mode for the given sensor. The first 
configuration screen the *'CoolOn'* threshold can be modified. Use *LEFT/RIGHT* arrows to change the temperature above which the relay for cooling will be activated. Then press *SEL*. The next screen modifies the *'CoolOff'* threshold, also set it with *LEFT/RIGHT* arrows. If the temperature falls below this threshold the MCU will deactivate the respective relay. The next screen is accessed by pressing *SEL* again.

The next two screens are dedicated to the heating part of the controller. *'HeatOn'* will turn on the heating relay if temperature 
falls below this threshold. When you are done press *SEL* to continue. *'HeatOff'* will turn off the heating relay when temperature rises above said threshold. Press *SEL* after this to continue to the offset calibration.

This screen allows to modify the offset of the temperature reading of the sensor. This gives the possibility to calibrate the sensor 
to a known reference temperature. By pressing *SEL* once more, all settings are saved and the program returns to the main screen.

NOTE: If heating and cooling parts of the controller overlap, cooling will take precedence.

### Configuration/Pinout
The configuration/pinout for the code can be modified easily in `/lib/fresca/fresca_pinout.h`. Look at the commented 
description of each line to know what they are used for.

Keypad is connected to an analog pin.
Each sensor is connected to a single digital I/O.
7-Segment displays CLK pins are connected to a single arduino digital pin.

### Celsius/Fahrenheit

To display temperature in fahrenheit set the **TEMP_FAHRENHEIT** define in `fresca.h` to other value than zero. Otherwise temperature is displayed in degrees celsius.

### Further customization/debugging

Many constants that define program behavior and debugging are defined in `fresca.h`, they are commented so you can easily find them and edit them!

### Resource usage

**'fresca.ino'** uses on an Arduino Mega 2560 for 8 processes sensors:
> Program memory: 16278 bytes (6.2% Full)  
> Data Memory: 1337 bytes (16.3% Full)  

Free memory in runtime is around 6900 bytes

### Libraries

This project uses libraries which can be found in the `lib` directory. These libraries are property of their respective owners
The third-party libraries used are:

* DFR_Key (for analog keypad) (modified)
* TM1637-1.1.0_7seg (for 7-segment display controller)
* OneWire (for DS1820)
* DHT from Adafruit industries (for DHT22)

Grab them and install them in your Arduino IDE (you don't need to do this if you are building from platformio or Arduino-Makefile)

### Why only one sensor per wire ?

As many of you know it is possible to accommodate many OneWire sensors in a single wire. Even though this represents a significant reduction in 
pin count, it makes things more complicated for the user. With one sensor per wire, the user can clearly identify what thresholds temperature 
reading belong to which sensor, and can even interchange sensors and replace them freely. Otherwise there would need to be a ROM matching mechanism 
for new/current sensors to respective thresholds. So the setup is easier and there is less confusion, and getting a high pin-count arduino is really 
cheap.