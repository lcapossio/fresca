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

*/

#ifndef DS1820_H
#define DS1820_H


#include <OneWire.h>

typedef int16_t DS1820_TEMP_DATA_TYPE; //Data type for temperature in fixed point format S12.4 (1 sign bit, 11 integer bits and 4 fractional bits)

////////////////////////////////////////
//DS1820 OneWire commands
#define CMD_START_TEMP       0x44 //Start temperature conversion
#define CMD_READ_SCRATCHPAD  0xBE //Read the whole scratchpad (9 bytes)
#define CMD_WRITE_SCRATCHPAD 0x4E //Write into scratchpad (Config register, TH and TL bytes)
#define CMD_COPY_SCRATCHPAD  0x48 //Copy scratchpad contents to EEPROM (Config register, TH and TL bytes)
////////////////////////////////////////

#ifndef DEBUG_DS1820
#define DEBUG_DS1820 false //Don't debug by default
#endif

//DS1820 class inherits methods from OneWire class
class DS1820 : OneWire
{
    private:
  
        HardwareSerial *dbgSerial;             //Serial object used for debug
        DS1820_TEMP_DATA_TYPE TempReading;     //Latest temperature reading
        DS1820_TEMP_DATA_TYPE SensorOffset;    //From TH and TL, latest stored calibration offset

    public:

        DS1820(uint8_t pin, HardwareSerial *serial_obj=NULL); //Constructor

        DS1820_TEMP_DATA_TYPE DS1820::GetTemp();
        DS1820_TEMP_DATA_TYPE DS1820::GetOffset();
        void DS1820::StartTemp();
        uint8_t DS1820::UpdateTemp(uint8_t chk_crc);
        void DS1820::WriteUserBytes(uint8_t config_reg, DS1820_TEMP_DATA_TYPE offset);
};

#endif