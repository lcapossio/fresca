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

*Header, constants and prototypes for 'fresca'
*Here some constants that control certain parts of the program can be changed
*Pinout is specified on the fresca.ino file
*Temperature display is defaulted to degrees celsius, but can be changed to fahrenheit with TEMP_FAHRENHEIT
 Celsius temperature display on 4-digit 7-segment LCD is two leftmost digits are whole part and two rightmost are fractional part
 Fahrenheit temperature display on 4-digit 7-segment LCD is three leftmost digits are whole part and the rightmost is fractional part
 All temperatures are represented internally with 16 bits signed 'Q11.4' fixed point format (celsius)
*********************************************************************************************
*/

#ifndef FRESCA_H
#define FRESCA_H

#include <stdint.h>

    typedef int16_t TEMP_DATA_TYPE;
    typedef int16_t HUM_DATA_TYPE;

    ////////////////////////////////////////
    //MACROS
    //Set timer1 registers
    #define SET_TIMER1(VAL)  OCR1A  = (unsigned) VAL;   /*Output compare match*/ \
                             TCNT1  = 0;                /*Reset timer count register*/
    //
    #define ROUND(x) ((x)>=0?(TEMP_DATA_TYPE)((x)+0.5):(TEMP_DATA_TYPE)((x)-0.5))
    //Convert to signed fixed point, max 16 bits, truncates result
    #define TEMPFLOAT2FIX(VAL_FP,SCALE) (TEMP_DATA_TYPE) ROUND(VAL_FP*(float)SCALE)
    ////////////////////////////////////////

    ////////////////////////////////////////
    //
    #define CRISTAL_FREQ_MHZ       16.0                     //Arduino operating frequency Floating point in Megahertz, user can modify this
    #define CRISTAL_FREQ_HZ        (CRISTAL_FREQ_MHZ*1e6)   //Arduino operating frequency Floating point in Hertz
    #define TIMER1_PRESCALE        256.0                    //Floating point, don't change
    #define TEMP_FRAC_BITS         4                        //Fractional bits for temperature representation
    #define HUM_FRAC_BITS         4                         //Fractional bits for humidity representation
    #define TEMP_SCALE             (1<<TEMP_FRAC_BITS)      //Scaling factor to transform floating point to fixed point
    
    //Define steps and max/min temperature values in deg celsius, user can modify this
    #define THRESHOLD_STEP_FP      0.25   //In floating point
    #define OFFSET_STEP_FP         0.0625 //In floating point
    #define MAX_TEMP_FP            30.0   //Max Temp for CoolOn
    #define MIN_TEMP_FP            5.0    //Min Temp for CoolOff
    #define MAX_OFF_TEMP_FP        1.0    //Max Temp for OffsetCalib
    #define MIN_OFF_TEMP_FP        1.0    //Min Temp for OffsetCalib (will be interpreted as negative)
    #define COOLON_DFLT_FP         25.0   //
    #define COOLOFF_DFLT_FP        24.0   //
    #define HEATON_DFLT_FP         24.0   //
    #define HEATOFF_DFLT_FP        25.0   //
    
    
    //Now calculate fixed point values, don't modify this
    #define THRESHOLD_STEP         TEMPFLOAT2FIX(THRESHOLD_STEP_FP,TEMP_SCALE)   //
    #define OFFSET_STEP            TEMPFLOAT2FIX(OFFSET_STEP_FP,TEMP_SCALE)      //
    #define MAX_TEMP               TEMPFLOAT2FIX(MAX_TEMP_FP,TEMP_SCALE)         //
    #define MIN_TEMP               TEMPFLOAT2FIX(MIN_TEMP_FP,TEMP_SCALE)         //
    #define MAX_OFF_TEMP           TEMPFLOAT2FIX(MAX_OFF_TEMP_FP,TEMP_SCALE)     //
    #define MIN_OFF_TEMP           TEMPFLOAT2FIX(MIN_OFF_TEMP_FP,TEMP_SCALE)     //
    #define COOLON_DFLT            TEMPFLOAT2FIX(COOLON_DFLT_FP,TEMP_SCALE)      //
    #define COOLOFF_DFLT           TEMPFLOAT2FIX(COOLOFF_DFLT_FP,TEMP_SCALE)     //
    #define HEATON_DFLT            TEMPFLOAT2FIX(HEATON_DFLT_FP,TEMP_SCALE)      //
    #define HEATOFF_DFLT           TEMPFLOAT2FIX(HEATOFF_DFLT_FP,TEMP_SCALE)     //
    

    ////////////////////////////////////////
    //Constants, user can modify this
    #define MAX_NUM_TEMP_SENSORS   8
    #define NUM_SENSORS            8            //One sensor per wire, SET TO THE ACTUAL NUMBER OF SENSORS
    #define NUM_7SEG               NUM_SENSORS  //Number of 7 segment displays (<=NUM_SENSORS)
    #define NUM_TEMP_COOLERS       NUM_SENSORS  //Number of cooling actuators (relays) (<=NUM_SENSORS)
    #define NUM_TEMP_HEATERS       NUM_SENSORS  //Number of heating actuators (relays) (<=NUM_SENSORS)
    #define DS1820_CONFIG_REG      0x7F         //12-bit resolution, no more options
    
    //Debug
    #define DEBUG_SENSORS          0       //!=0 serial debug messages are enabled (Key press messages)
    #define DEBUG_KEYS             0       //!=0 serial debug messages are enabled (Sensor data and related messages)
    #define DEBUG_PERF             0       //!=0 serial debug messages are enabled (Performance and RAM usage)
    //
    #define TEMP_FAHRENHEIT        0       //!=0 temperature is displayed in fahrenheit, otherwise in celsius
    #define USE_CRC                1       //!=0 DS1820 CRC check is enabled
    #define MAX_BUF_CHARS         64       //Max. characters for print buffer
    #define TEMP_POLL_SEC        750       //Temperature polling in milliseconds
    #define LCD_WIDTH             16       //LCD horizontal length
    #define LCD_HEIGHT             2       //LCD vertical length
    #define RELAY_ACTIVE           0       //0: Active LOW relays, 1 active HIGH relays
    #define KEYPAD_REFRESH_RATE   20       //Sets the sample rate of the keypad at once every x milliseconds.
    #define TIMER_20MS            ((CRISTAL_FREQ_HZ/TIMER1_PRESCALE)*0.02)  //OCR1A value
    #define TIMER_100MS           ((CRISTAL_FREQ_HZ/TIMER1_PRESCALE)*0.1)   //OCR1A value
    #define TIMER_250MS           ((CRISTAL_FREQ_HZ/TIMER1_PRESCALE)*0.25)  //OCR1A value
    #define TIMER_500MS           ((CRISTAL_FREQ_HZ/TIMER1_PRESCALE)*0.5)   //OCR1A value
    #define INIT_DELAY            2000                   //Time to before starting main loop in milliseconds
    ////////////////////////////////////////

    ////////////////////////////////////////
    //EEPROM, don't modify this
    #define EEPROM_MAGIC_NUM_ADDR 0             //Byte variable stored in this location indicates EEPROM has been written previously
    #define EEPROM_MAGIC_NUM_VALUE 0x5A
    #define EEPROM_START_ADDR 1
    #define EEPROM_BLOCKSIZE  sizeof(TEMP_DATA_TYPE)*4 //Each block contains: CoolOn, CoolOff, HeatOn and HeatOff
    #define EEPROM_ADDR_INCR  sizeof(TEMP_DATA_TYPE)
    ////////////////////////////////////////

    ////////////////////////////////////////
    //Function prototypes
    void read_temp_sensors(); //Read all temperature sensors and update global temperature variables
    void main_menu();         //Main menu
    ////////////////////////////////////////
    
#endif