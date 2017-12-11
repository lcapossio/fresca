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

#ifndef FRESCA_SENS_H
#define FRESCA_SENS_H

#include <fresca.h>
#include <Adafruit_Sensor.h>
#include <ds1820.h>
#include <DHT.h>

typedef int16_t SENS_TEMP_DATA_TYPE; //Data type for temperature in fixed point format S12.4 (1 sign bit, 11 integer bits and 4 fractional bits)
typedef int16_t SENS_HUM_DATA_TYPE;  //Data type for humidity in fixed point format S12.4 (1 sign bit, 11 integer bits and 4 fractional bits)

#ifdef  DEBUG_SENS
#define DBG_SENS true
#else
#define DBG_SENS false //Don't debug by default
#endif

#define MAX_SENSORS MAX_NUM_TEMP_SENSORS

#define TEMP_FIX_POS     4    //Position of the fixed point
#define HUMIDITY_FIX_POS 4    //Position of the fixed point

enum class SensorType_t       : uint8_t {FRESCA_SENS_DS1820=0, FRESCA_SENS_DHT22=1};
enum class SensorStatus_t     : uint8_t {FRESCA_SENS_OK=0, FRESCA_SENS_ERROR=1, FRESCA_SENS_CRC_ERROR=2};

//fresca_sensor class
class fresca_sensor
{
    private:
    
        HardwareSerial *_dbgSerial;                   //Serial object used for debug
        void *          _sensor[MAX_SENSORS];         //Sensor class pointer array
        SensorType_t    _sensorType[MAX_SENSORS];     //Type of sensor (DS1820 or DHT22)
        SensorStatus_t  _sensorStatus[MAX_SENSORS];   //Type of sensor (DS1820 or DHT22)
        uint8_t         _numSensors;                  //

    public:

        fresca_sensor(uint8_t numSensors, const SensorType_t *sensorType, const uint8_t *pin, HardwareSerial *serial_obj=NULL); //Constructor

        SENS_HUM_DATA_TYPE  GetHumidity(uint8_t sensor_idx); //Get humidity
        SENS_TEMP_DATA_TYPE GetTemp(uint8_t sensor_idx);
        SENS_TEMP_DATA_TYPE GetTempOffset(uint8_t sensor_idx);
        uint8_t             SetTempOffset(uint8_t sensor_idx, SENS_TEMP_DATA_TYPE offset, uint8_t config_reg);
        SENS_TEMP_DATA_TYPE SetSensConfig(uint8_t sensor_idx, uint8_t config);
        
        uint8_t             GetHumiditySupport(uint8_t sensor_idx);
        SensorStatus_t      GetStatus(uint8_t sensor_idx);
        SensorType_t        GetType(uint8_t sensor_idx);
};

#endif