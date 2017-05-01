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
    
    if (TEMP_FAHRENHEIT)
    {
        temp     = celsius2fahrenheit(temp);
    }
    SignBit  = (temp < 0) ? true : false;      //test most sig bit
    Whole    = SignBit ? -temp : temp;         //Complement if negative
    Fract    = ((Whole&0xF)*100)>>4;           //Leave only the last 2 decimal fractional digits
    Whole    = Whole>>4;                       //Divide by 16 to get the whole part

    //Print in the second row
    snprintf(print_buf, LCD_WIDTH+1, "%c%02u.%02u\xDF%c %s", SignBit ? '-':'+', Whole, Fract, (TEMP_FAHRENHEIT==0)?'C':'F', show_error ? "  ERR!     " : "          "); //0xDF is *deg* in the LCD char set
    lcd.setCursor(0,1);
    lcd.print(print_buf);
    
    //Always successful
    return true;
}

//Turn cooling on/off
inline bool SwitchCooling(uint8_t sensor, bool state)
{
    if (state)
    {
        //Turn on cooling
        digitalWrite(g_CoolSwitch[sensor], (RELAY_ACTIVE!=0) ? HIGH : LOW);
    }
    else
    {
        //Turn off cooling
        digitalWrite(g_CoolSwitch[sensor], (RELAY_ACTIVE!=0) ? LOW : HIGH);
    }
    
    //Always successful
    return true;
}


int UpdateCoolOn(int currVal, int CoolOffVal, bool inc_dec)
{
    //Step
    if (inc_dec)
    {
        //Increment
        currVal+=THRESHOLD_STEP;
    }
    else
    {
        //Decrement
        currVal-=THRESHOLD_STEP;
    }
    
    //Check limits
    if (currVal > MAX_TEMP)
    {
        currVal = MAX_TEMP;
    }
    else if (currVal < CoolOffVal+THRESHOLD_STEP)
    {
        currVal = CoolOffVal+THRESHOLD_STEP;
    }
    
    return currVal;
}

int UpdateCoolOff(int currVal, int CoolOnVal, bool inc_dec)
{
    //Step
    if (inc_dec)
    {
        //Increment
        currVal+=THRESHOLD_STEP;
    }
    else
    {
        //Decrement
        currVal-=THRESHOLD_STEP;
    }
    
    //Check limits
    if (currVal < MIN_TEMP)
    {
        currVal = MIN_TEMP;
    }
    else if (currVal > CoolOnVal-THRESHOLD_STEP)
    {
        currVal = CoolOnVal-THRESHOLD_STEP;
    }
    
    return currVal;
}

inline uint8_t SensorNext(uint8_t currSensor)
{
    currSensor += 1;
    if (currSensor > NUM_DS1820_SENSORS-1)
    {
        currSensor = 0;
    }
    
    return currSensor;
}

inline uint8_t SensorPrev(uint8_t currSensor)
{
    currSensor -= 1;
    if (currSensor > NUM_DS1820_SENSORS-1)
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

//Write user bytes into DS1820 and copy it to the EEPROM
//Offset will be stored in TH and TL register of DS1820
void ds1820_WriteUserBytes(OneWire *sensor, uint8_t config_reg, int16_t offset)
{
    uint8_t TH, TL;
    
    //Mask offset into separate bytes
    TH = (offset >> 8) & 0xFF;
    TL = offset & 0xFF;
    
    //
    sensor->reset();
    sensor->skip();         //SkipROM command, we only have one sensor per wire
    sensor->write(0x4E);    //Write Scratchpad
    
    //Now write 3 bytes in this order TH, TL and configuration register
    sensor->write(TH);          //Write byte
    sensor->write(TL);          //Write byte
    sensor->write(config_reg);  //Write byte
    
    //Now transfer the scratchpad contents to EEPROM
    sensor->reset();
    sensor->skip();         //SkipROM command, we only have one sensor per wire
    sensor->write(0x48);    //Copy Scratchpad

    return;
}

//Celsius comes in Q11.4
int16_t celsius2fahrenheit(int16_t celsius)
{
    int16_t fahrenheit;
    
    //(9/5*celsius) + 32
    //or (1.8*celsius) + 32
    fahrenheit = (TEMPFLOAT2FIX(1.8,TEMP_SCALE) * celsius) >> (TEMP_FRAC_BITS);
    fahrenheit += TEMPFLOAT2FIX(32.0,TEMP_SCALE);
    
    // //Debug
    // Serial.print("Celsius: ");Serial.print(celsius, DEC);Serial.print(" ***");Serial.println();
    // Serial.print("Fahrenheit: ");Serial.print(fahrenheit, DEC);Serial.print(" ***");Serial.println();
    
    return fahrenheit;
}

////////////////////////////////////////////////////////////////////
//Measure RAM usage, for debug only, by Bill Earl, from adafruit.com
int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}