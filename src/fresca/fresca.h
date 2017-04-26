/*
*********************************************************************************************
** Author: Leonardo Capossio
** Project: 'fresca'
** Description:
**             
**             
**             
**             
**             
**             
**             
**             
**             
**             
**             
**             
**             
**             
*********************************************************************************************
*/

#ifndef FRESCA_H
#define FRESCA_H


    #define MAX_NUM_DS1820_SENSORS 8
    #define NUM_DS1820_SENSORS     8                           //One sensor per wire
    #define NUM_DISP_7SEG          NUM_DS1820_SENSORS          //
    #define NUM_COOL_RELAYS        NUM_DS1820_SENSORS          //

    //Constants
    #define DO_DEBUG 1              //!=0 debug is enabled
    #define USE_CRC  1              //!=0 CRC check is enabled
    #define MAX_BUF_CHARS  64
    #define MAX_TEMP 30*16
    #define MIN_TEMP 5*16
    #define DEG_0_5  8              //0.5deg*16
    #define TEMP_POLL_SEC 0.8       //Temperature polling in seconds
    #define LCD_WIDTH 16
    #define LCD_HEIGHT 2
    #define RELAY_ACTIVE 0          //0: Active LOW relays, 1 active HIGH relays
    #define KEYPAD_REFRESH_RATE 40  //Sets the sample rate of the keypad at once every x milliseconds.

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
    extern TM1637Display *g_disp7seg[NUM_DISP_7SEG];        //7segment displays (TM1637)
    extern OneWire *g_ds1820[NUM_DS1820_SENSORS];           //DS18B20 Digital temperature sensor
    extern const byte g_CoolSwitch[MAX_NUM_DS1820_SENSORS]; //Cooling actuators (Relays)
    extern LiquidCrystal lcd;                               //LCD 16x2 based on the Hitachi HD44780 controller
    extern DFR_Key keypad;                                  //Analog Keypad on the LCD
    ////////////////////////////////////////
    
#endif