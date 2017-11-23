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

  fresca_sensor class manages an array of different types of temperature/humidity sensors

*/

#include "fresca_sensor.h"
#include <DHT.h>
#include "ds1820.h"

//Constructor
fresca_sensor::fresca_sensor(uint8_t numSensors, const SensorType_t *sensorType, const uint8_t *pin, HardwareSerial *serial_obj)
{
    _dbgSerial  = serial_obj;
    _numSensors = numSensors;
    
    for (uint8_t sensor_idx = 0; sensor_idx < numSensors; sensor_idx++)
    {
        switch (sensorType[sensor_idx])
        {
            //DHT22
            case SensorType_t::FRESCA_SENS_DHT22:
                //Create object
                _sensor[sensor_idx] = new DHT(pin[sensor_idx],DHT22);
                //Start sensor
                ((DHT *) _sensor[sensor_idx])->begin();
                break;
            case SensorType_t::FRESCA_SENS_DS1820:
            default:
                //Create object
                _sensor[sensor_idx] = new DS1820(pin[sensor_idx],serial_obj);
                //Kick start temperature conversion
                ((DS1820 *) _sensor[sensor_idx])->StartTemp();
                break;
        }
        _sensorType[sensor_idx] = sensorType[sensor_idx]; // Store sensor type in internal variable
    }
    
    return;
}

//Gets latest temperature reading, if failed will return previous temperature reading
SENS_TEMP_DATA_TYPE fresca_sensor::GetTemp(uint8_t sensor_idx)
{
    SENS_TEMP_DATA_TYPE TempReading;
    switch (_sensorType[sensor_idx])
    {
        //DHT22
        case SensorType_t::FRESCA_SENS_DHT22:
            float TempReadingDHT22;
            //read data from DHT22
            TempReadingDHT22 = ((DHT *) _sensor[sensor_idx])->readTemperature();
            //Convert to fixed point
            TempReading = (SENS_TEMP_DATA_TYPE) (TempReadingDHT22  * (1<<TEMP_FIX_POS));
            //Set sensor status (DHT22 is always OK, since library doesn't support error conditions)
            _sensorStatus[sensor_idx] = SensorStatus_t::FRESCA_SENS_OK;
            break;
        //DS1820
        case SensorType_t::FRESCA_SENS_DS1820:
        default:
            uint8_t ds1820_status;
            //Reads the temperature from the sensor and updates offset saved in sensor
            ds1820_status = ((DS1820 *) _sensor[sensor_idx])->UpdateTemp(USE_CRC);
            //Restart temperature conversion
            ((DS1820 *) _sensor[sensor_idx])->StartTemp();
            //Will return old temperature if it failed
            TempReading = ((DS1820 *) _sensor[sensor_idx])->GetTemp();

            //Set sensor status
            _sensorStatus[sensor_idx] = (ds1820_status) ? SensorStatus_t::FRESCA_SENS_OK : SensorStatus_t::FRESCA_SENS_ERROR;

            break;
    }
    
    return TempReading;
}

//Gets latest offset reading from sensor non-volatile storage
SENS_TEMP_DATA_TYPE fresca_sensor::GetTempOffset(uint8_t sensor_idx)
{
    SENS_TEMP_DATA_TYPE SensorOffset;
    switch (_sensorType[sensor_idx])
    {
        //DHT22
        case SensorType_t::FRESCA_SENS_DHT22:
            //No support for DHT22 offset yet
            SensorOffset = 0;
            break;
        //DS1820
        case SensorType_t::FRESCA_SENS_DS1820:
        default:
            SensorOffset = ((DS1820 *) _sensor[sensor_idx])->GetOffset();
            break;
    }
    
    return SensorOffset;
}


SENS_HUM_DATA_TYPE  fresca_sensor::GetHumidity(uint8_t sensor_idx) //Get humidity
{
    SENS_HUM_DATA_TYPE HumReading;
    switch (_sensorType[sensor_idx])
    {
        //DHT22
        case SensorType_t::FRESCA_SENS_DHT22:
            float DHT_Hum;
            //Read data from DHT22
            DHT_Hum = ((DHT *) _sensor[sensor_idx])->readHumidity();
            //Convert to fixed point
            HumReading = (SENS_HUM_DATA_TYPE) (DHT_Hum * (1<<HUMIDITY_FIX_POS));
            break;
        //DS1820
        case SensorType_t::FRESCA_SENS_DS1820:
        default:
            //Humidity reading not supported
            HumReading=0;
            break;
    }
    
    return HumReading;
}

uint8_t fresca_sensor::SetTempOffset(uint8_t sensor_idx, SENS_TEMP_DATA_TYPE offset, uint8_t config_reg)
{
    switch (_sensorType[sensor_idx])
    {
        //DHT22
        case SensorType_t::FRESCA_SENS_DHT22:
            //No support for DHT22 offset yet
            break;
        //DS1820
        case SensorType_t::FRESCA_SENS_DS1820:
        default:
            //Have to find how to record which config reg is
            ((DS1820 *) _sensor[sensor_idx])->WriteUserBytes(config_reg,offset);
            break;
    }
    
    return true;
}

SensorStatus_t fresca_sensor::GetStatus(uint8_t sensor_idx)
{
  return _sensorStatus[sensor_idx];
}

SensorType_t fresca_sensor::GetType(uint8_t sensor_idx)
{
  return _sensorType[sensor_idx];
}