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
            fresca utility functions
            
            
            
            
            
            
            
            
            
            
            
            
            
            
*********************************************************************************************
*/

#include <LiquidCrystal.h>
#include <DFR_Key.h>
#include "fresca.h"
#include "utils.h"

//Delays in milliseconds, even when interrupts are disabled
void delay_noInterrupts(uint16_t millis)
{
    for (uint16_t i=0; i < millis;i++)
    {
        delayMicroseconds(1000);
    }
    return;
}

//Prints temperature in the second row of the LCD
bool PrintTempLCD(int16_t temp, bool show_error)
{
    // Separate off the whole and fractional portions, since sprintf doesn't support printing floats!!!
    char print_buf[MAX_BUF_CHARS];
    bool SignBit;
    int16_t Whole, Fract;
    
    SignBit  = (temp < 0) ? true : false;      //test most sig bit
    Whole    = SignBit ? -temp : temp;         //Complement if negative
    Fract    = ((Whole&0xF)*100)>>4;           //Leave only the last 2 decimal fractional digits
    Whole    = Whole>>4;                       //Divide by 16 to get the whole part

    //Print in the second row
    snprintf(print_buf, LCD_WIDTH+1, "%c%02u.%02u\xDF C%s", SignBit ? '-':'+', Whole, Fract, show_error ? "  ERR!     " : "          "); //0xDF is *deg* in the LCD char set
    lcd.setCursor(0,1);
    lcd.print(print_buf);
    
    //Always successful
    return true;
}

bool SwitchCooling(byte sensor, bool state)
{
    if (state)
    {
        //Turn on cooling
        digitalWrite(g_CoolSwitch[sensor], HIGH);
    }
    else
    {
        //Turn off cooling
        digitalWrite(g_CoolSwitch[sensor], LOW);
    }
    
    //Always successful
    return true;
}


int UpdateCoolOn(int currVal, int CoolOffVal, bool inc_dec)
{
    //0.5deg Steps
    if (inc_dec)
    {
        //Increment
        currVal+=DEG_0_5;
    }
    else
    {
        //Decrement
        currVal-=DEG_0_5;
    }
    
    //Check limits
    if (currVal > MAX_TEMP)
    {
        currVal = MAX_TEMP;
    }
    else if (currVal < CoolOffVal+DEG_0_5)
    {
        currVal = CoolOffVal+DEG_0_5;
    }
    
    return currVal;
}

int UpdateCoolOff(int currVal, int CoolOnVal, bool inc_dec)
{
    //0.5deg Steps
    if (inc_dec)
    {
        //Increment
        currVal+=DEG_0_5;
    }
    else
    {
        //Decrement
        currVal-=DEG_0_5;
    }
    
    //Check limits
    if (currVal < MIN_TEMP)
    {
        currVal = MIN_TEMP;
    }
    else if (currVal > CoolOnVal-DEG_0_5)
    {
        currVal = CoolOnVal-DEG_0_5;
    }
    
    return currVal;
}

int SensorNext(int currSensor)
{
    currSensor += 1;
    if (currSensor > NUM_DS1820_SENSORS-1)
    {
        currSensor = 0;
    }
    
    return currSensor;
}

int SensorPrev(int currSensor)
{
    currSensor -= 1;
    if (currSensor < 0)
    {
        currSensor = NUM_DS1820_SENSORS-1;
    }
    
    return currSensor;
}

//Returns true if select key is pressed
bool SelectKeyPressed()
{
    int tempKey;

    do
    {
        tempKey  = keypad.getKey();
    } while (tempKey == SAMPLE_WAIT);
    
    if (tempKey == SELECT_KEY) return true;
    
    return false;
}

//Set Timer1
void setTimer1(unsigned value)
{
    OCR1A  = value; //Output compare match
    TCNT1  = 0;     //Reset timer count register
}

////////////////////////////////////////////////////////////////////
//Measure RAM usage, for debug only, by Bill Earl, from adafruit.com
int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}