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

    #include <LiquidCrystal.h>
    #include <DFR_Key.h>
    #include "fresca.h"

    ////////////////////////////////////////
    //Function declarations
    void delay_noInterrupts(uint16_t millis);                                                       //Delays in milliseconds, even when interrupts are disabled
    bool PrintTempLCD(TEMP_DATA_TYPE temp, bool show_error, LiquidCrystal *lcd);                    //Print temperature in second row of LCD
    bool PrintHumidityLCD(HUM_DATA_TYPE temp, bool show_error, LiquidCrystal *lcd);                 //Print humidity in second row of LCD
    uint8_t SensorNext(uint8_t currSensor);                                                         //Get next sensor (and wrap around)
    uint8_t SensorPrev(uint8_t currSensor);                                                         //Get previous sensor (and wrap around)
    bool SelectKeyPressed(DFR_Key *keypad);                                                         //Returns true if select key is pressed
    TEMP_DATA_TYPE celsius2fahrenheit(TEMP_DATA_TYPE celsius);                                      //Converts temp from celsius into fahrenheit
    int  freeRam();                                                                                 //Measure available RAM, for debug only, by Bill Earl, from adafruit.com
    ////////////////////////////////////////
    
#endif