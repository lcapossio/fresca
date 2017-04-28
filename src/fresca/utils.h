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
    //Function declarations
    void delay_noInterrupts(uint16_t millis);                       //Delays in milliseconds, even when interrupts are disabled
    bool PrintTempLCD(int16_t temp, bool show_error);               //Print temperature in second row of LCD
    bool SwitchCooling(byte sensor, bool state);                    //Turn on or off cooling
    int  UpdateCoolOn(int currVal, int CoolOffVal, bool inc_dec);   //Update CoolOn threshold
    int  UpdateCoolOff(int currVal, int CoolOnVal, bool inc_dec);   //Update CoolOff threshold
    int  SensorNext(int currSensor);                                //Get next sensor
    int  SensorPrev(int currSensor);                                //Get previous sensor
    bool SelectKeyPressed();                                        //Returns true if select key is pressed
    void setTimer1(unsigned value);
    int  freeRam();                                                 //Measure RAM usage, for debug only, by Bill Earl, from adafruit.com
    ////////////////////////////////////////
    
#endif