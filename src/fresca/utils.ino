/*
*********************************************************************************************
** Author: Leonardo Capossio
** Project: 'fresca'
** Description: fresca utility functions
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

#include <LiquidCrystal.h>
#include "fresca.h"
#include "utils.h"

//Delays in milliseconds, even when interrupts are disabled
void Delay_noInterrupts(uint16_t millis)
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

//Measure RAM usage, for debug only, by Bill Earl, from adafruit.com
int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}