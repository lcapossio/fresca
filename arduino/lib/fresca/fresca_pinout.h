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

Fresca implementation related/board specific data

*********************************************************************************************
*/

#ifndef FRESCA_PINOUT_H
#define FRESCA_PINOUT_H

#include "fresca.h"
#include "fresca_sensor.h"

////////////////////////////////////////
// ****** DEFINE PINOUT HERE ****** PINOUT_LINE, user can modify this
//Define pinout for MAXIMUM number of sensors (MAX_NUM_TEMP_SENSORS), if less sensors are actually implemented (NUM_SENSORS), the rest of the pins will not be used or touched at all
const uint8_t gc_7seg_dio_pins[MAX_NUM_TEMP_SENSORS]   = {7,14,15,16,17,43,44,20};   //All TM1637 DIO are specified here
const uint8_t gc_7seg_clk_pins                         = {8};                        //One clock for all TM1637 displays
const uint8_t g_CoolerPins[MAX_NUM_TEMP_SENSORS]       = {9,21,22,23,24,25,26,27};   //Contains pin number for relay
const uint8_t g_CoolerEn[MAX_NUM_TEMP_SENSORS]         = {1,1,1,1,1,1,1,1};          //Enable for Cooler actuator (otherwise temperature control is not implemented)
const uint8_t g_HeaterPins[MAX_NUM_TEMP_SENSORS]       = {35,36,37,38,39,40,41,42};  //Contains pin number for relay
const uint8_t g_HeaterEn[MAX_NUM_TEMP_SENSORS]         = {1,1,1,1,1,1,1,1};          //Enable for Heater actuator (otherwise temperature control is not implemented)
const uint8_t gc_lcd_pins[6]                           = {12, 11, 5, 4, 3, 2};       //LCD 16x2 based on the Hitachi HD44780 controller (rs, enable, d4, d5, d6, d7)
const uint8_t gc_keypad_pins                           = 0;                          //Analog Keypad on the LCD PCB (has to be an analog pin)
const uint8_t gc_temp_sens_pins[MAX_NUM_TEMP_SENSORS]  = {6,28,29,30,31,32,33,34};   //Temperature sensors pins (one sensor per wire)
const SensorType_t gc_temp_sens_type[MAX_NUM_TEMP_SENSORS]  = {                      //Specify what sensor is hooked to each index (matched up with the pins)
                                                               SensorType_t::FRESCA_SENS_DS1820,
                                                               SensorType_t::FRESCA_SENS_DS1820,
                                                               SensorType_t::FRESCA_SENS_DS1820,
                                                               SensorType_t::FRESCA_SENS_DS1820,
                                                               SensorType_t::FRESCA_SENS_DS1820,
                                                               SensorType_t::FRESCA_SENS_DS1820,
                                                               SensorType_t::FRESCA_SENS_DS1820,
                                                               SensorType_t::FRESCA_SENS_DS1820
                                                              };   //Temperature sensors: DS18B20 or DHT22
                                                              
#endif