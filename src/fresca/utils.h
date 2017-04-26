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

#ifndef FRESCA_UTILS_H
#define FRESCA_UTILS_H

    ////////////////////////////////////////
    //Function declarations
    void Delay_noInterrupts(uint16_t millis);                       //Delays in milliseconds, even when interrupts are disabled
    bool PrintTempLCD(int16_t temp, bool show_error);               //Print temperature in second row of LCD
    bool SwitchCooling(byte sensor, bool state);                    //Turn on or off cooling
    int  UpdateCoolOn(int currVal, int CoolOffVal, bool inc_dec);   //Update CoolOn threshold
    int  UpdateCoolOff(int currVal, int CoolOnVal, bool inc_dec);   //Update CoolOff threshold
    int  SensorNext(int currSensor);                                //Get next sensor
    int  SensorPrev(int currSensor);                                //Get previous sensor
    int  freeRam();                                                 //Measure RAM usage, for debug only, by Bill Earl, from adafruit.com
    ////////////////////////////////////////
    
#endif