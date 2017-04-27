/*
    'fresca' project, temperature control for making beer!
    Copyright (C) 2017  Leonardo M. Capossio

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
*********************************************************************************************
Author: Leonardo Capossio
Project: 'fresca'
Description:
            Header, constants and prototypes for 'fresca'
            Here some constants that control certain parts of the program can be changed
            Pinout is specified on the fresca.ino file
            
            
            
            
            
            
            
            
            
            
            
            
            
*********************************************************************************************
*/

#ifndef FRESCA_H
#define FRESCA_H

    ////////////////////////////////////////
    //Constants
    #define MAX_NUM_DS1820_SENSORS 8
    #define NUM_DS1820_SENSORS     8                           //One sensor per wire
    
    #define DO_DEBUG 0              //!=0 serial debug messages are enabled
    #define USE_CRC  0              //!=0 CRC check is enabled
    #define MAX_BUF_CHARS  64       //Max. characters for print buffer
    #define MAX_TEMP 30*16          //Max Temp for CoolOn
    #define MIN_TEMP 5*16           //Min Temp for CoolOff
    #define DEG_0_5  8              //0.5deg*16
    #define TEMP_POLL_SEC 0.8       //Temperature polling in seconds
    #define LCD_WIDTH 16            //LCD horizontal size
    #define LCD_HEIGHT 2            //LCD vertical size
    #define RELAY_ACTIVE 0          //0: Active LOW relays, 1 active HIGH relays
    #define KEYPAD_REFRESH_RATE 20  //Sets the sample rate of the keypad at once every x milliseconds.
    ////////////////////////////////////////

    ////////////////////////////////////////
    //EEPROM
    #define EEPROM_MAGIC_VAR_ADDR 0             //Byte variable stored in this location indicates EEPROM has been written previously
    #define EEPROM_MAGIC_VAR_VALUE 0x5A
    #define EEPROM_START_ADDR 1
    #define EEPROM_BLOCKSIZE  sizeof(int16_t)*3 //Each block contains: CoolOn, CoolOff, Calibration Offset
    #define EEPROM_ADDR_INCR  sizeof(int16_t)
    ////////////////////////////////////////
    
    
    ////////////////////////////////////////
    //Global variables
    extern byte     g_showtempLCD;                          //Set to the Sensor number you want to display on the LCD (0: sensor0)
    extern int16_t  g_TempReading[NUM_DS1820_SENSORS];      //TempReading for every sensor
    extern int16_t  g_CoolOnThresh[NUM_DS1820_SENSORS];     //CoolOnThreshold for every sensor
    extern int16_t  g_CoolOffThresh[NUM_DS1820_SENSORS];    //CoolOffThreshold for every sensor
    extern int16_t  g_OffsetSensor[NUM_DS1820_SENSORS];     //Offset calibration for every sensor
    
    //Objects (Peripherals)
    extern TM1637Display *g_disp7seg[NUM_DS1820_SENSORS];   //7segment displays (TM1637)
    extern OneWire *g_ds1820[NUM_DS1820_SENSORS];           //DS18B20 Digital temperature sensor
    extern const byte g_CoolSwitch[NUM_DS1820_SENSORS];     //Cooling actuators (Relays)
    extern LiquidCrystal lcd;                               //LCD 16x2 based on the Hitachi HD44780 controller
    extern DFR_Key keypad;                                  //Analog Keypad on the LCD
    ////////////////////////////////////////
    
    
    ////////////////////////////////////////
    //Function prototypes
    void read_temp_sensors(); //Read all temperature sensors and update global temperature variables
    ////////////////////////////////////////
    
#endif