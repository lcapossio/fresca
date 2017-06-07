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

#include "ds1820.h"

//Constructor
DS1820::DS1820(uint8_t pin, HardwareSerial *serial_obj) : OneWire(pin)
{
    _dbgSerial = serial_obj; //Serial object for debugging (must be previously initialized)
    
    return;
}

//Reads the temperature from the sensor and updates offset saved in sensor
uint8_t DS1820::UpdateTemp(uint8_t chk_crc)
{
    uint8_t data[9];
    uint8_t present;
    uint8_t crc_err;
    DS1820_TEMP_DATA_TYPE HighByte, LowByte;
    
    present = reset();
    
    if (present)
    {
        ///////////////////////////////////////////////////
        //If sensor is present, read and update temperature
        skip();                        //SkipROM command, we only have one sensor per wire
        write(CMD_READ_SCRATCHPAD);    //Read Scratchpad
        if (DBG_DS1820) { _dbgSerial->print("Data sensor = "); }
        
        uint8_t data_zero_cnt=0;
        for (uint8_t i=0; i < 9; i++)
        {
            // Read 9 bytes
            data[i] = read();
            if (data[i] == 0) data_zero_cnt++;
            if (DBG_DS1820)
            {
                _dbgSerial->print(" 0x");
                _dbgSerial->print(data[i], HEX);
            }
        }

        //////////////
        //Process data
        crc_err=false;

        if (data_zero_cnt==9)
        {
            //If all bytes are zero, data is wrong, probably no sensor present
            present=false;
            if (DBG_DS1820) { _dbgSerial->print(" --- ERROR, CAN'T FIND SENSOR ***");_dbgSerial->println();}
        }
        else
        {
            if (chk_crc)
            {
                //Check CRC
                crc_err = (OneWire::crc8(data, 9)==0) ? false : true; //CRC equal 0 over the whole scratchpad memory means CRC is correct, otherwise CRC failed
            }
            if ( crc_err == false )
            {
                //Everything OK!!! Update temperature and offset
                if (DBG_DS1820) {_dbgSerial->print(" - CRC OK");_dbgSerial->println();}
                
                //Update temperature
                LowByte      = data[0]; //Temp low byte
                HighByte     = data[1]; //Temp high byte
                _TempReading = (HighByte << 8) + LowByte;
            }
            else
            {
                //There was a CRC error, don't update temperature
                if (DBG_DS1820) {_dbgSerial->print(" - CRC ERR");_dbgSerial->println();}
            }
            
            //Update Offset calibration from DS1820 scratchpad
            HighByte      = data[2]; //Offset high byte
            LowByte       = data[3]; //Offset low byte
            _SensorOffset = (HighByte << 8) + LowByte;
        }
    }
    else
    {
        if (DBG_DS1820) { _dbgSerial->print("***CAN'T FIND SENSOR ***");_dbgSerial->println();}
    }
    
    if ((crc_err==false) && present)
         return true; //Correct reading
    
    return false;  //CRC error or sensor not present
}

//Gets latest temperature reading
DS1820_TEMP_DATA_TYPE DS1820::GetTemp()
{
    return _TempReading;
}

//Gets latest offset reading
DS1820_TEMP_DATA_TYPE DS1820::GetOffset()
{
    return _SensorOffset;
}

//Start temperature conversion
void DS1820::StartTemp()
{
    reset();                  // Reset OneWire bus
    skip();                   // SkipROM command, we only have one sensor per wire
    write(CMD_START_TEMP,0);  // Command start temperature conversion
}

//Write user bytes into DS1820 and copy it to the EEPROM
//Offset will be stored in TH and TL register of DS1820
void DS1820::WriteUserBytes(uint8_t config_reg, DS1820_TEMP_DATA_TYPE offset)
{
    uint8_t TH, TL;
    
    //Mask offset into separate bytes
    TH = (offset >> 8) & 0xFF;
    TL = offset & 0xFF;
    
    //
    reset();                        // Reset OneWire bus
    skip();                         // SkipROM command, we only have one g_ds1820 per wire
    write(CMD_WRITE_SCRATCHPAD);    // Write Scratchpad
    
    //Now write 3 bytes in this order TH, TL and configuration register
    write(TH);          //Write byte
    write(TL);          //Write byte
    write(config_reg);  //Write byte
    
    //Now transfer the scratchpad contents to EEPROM
    reset();                       // Reset OneWire bus
    skip();                        // SkipROM command, we only have one g_ds1820 per wire
    write(CMD_COPY_SCRATCHPAD);    // Copy Scratchpad to EEPROM

    return;
}