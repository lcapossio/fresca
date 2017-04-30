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
            
            
            
            
            
            
            
            
            
            
            
            
            
            
*********************************************************************************************
*/

#ifndef FRESCA_UTILS_H
#define FRESCA_UTILS_H

    ////////////////////////////////////////
    //Macros
    #define SET_TIMER1(VAL)  OCR1A  = (unsigned) VAL;   /*Output compare match*/ \
                             TCNT1  = 0;                /*Reset timer count register*/
    ////////////////////////////////////////

    ////////////////////////////////////////
    //Function declarations
    void delay_noInterrupts(uint16_t millis);                       //Delays in milliseconds, even when interrupts are disabled
    bool PrintTempLCD(int16_t temp, bool show_error);               //Print temperature in second row of LCD
    bool SwitchCooling(byte sensor, bool state);                    //Turn cooling on/off
    int  UpdateCoolOn(int currVal, int CoolOffVal, bool inc_dec);   //Update CoolOn threshold
    int  UpdateCoolOff(int currVal, int CoolOnVal, bool inc_dec);   //Update CoolOff threshold
    byte SensorNext(byte currSensor);                               //Get next sensor (and wrap around)
    byte SensorPrev(byte currSensor);                               //Get previous sensor (and wrap around)
    bool SelectKeyPressed();                                        //Returns true if select key is pressed
    void ds1820_WriteUserBytes(OneWire *, byte, int16_t);           //Write DS1820 user bytes into EEPROM
    int16_t celsius2fahrenheit(int16_t celsius);                    //Converts temp from celsius into fahrenheit
    int  freeRam();                                                 //Measure RAM usage, for debug only, by Bill Earl, from adafruit.com
    ////////////////////////////////////////
    
#endif