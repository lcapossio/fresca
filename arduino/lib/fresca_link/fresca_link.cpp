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

#include "fresca_link.h"
// #include <SoftwareSerial.h>

// HardwareSerial Serial1;
// SoftwareSerial Serial_test(18,19);

void setup_fresca_link()
{
  Serial1.begin(57600);
  return;
}

//Send temperature data
void send_temp_link(uint8_t payload_len, uint8_t *payload)
{
    tx_packet(LINK_TEMP_CMD,payload_len,payload);
    return;
}

//Request config packet, if any, from host
void send_request_config(uint8_t config_size,uint8_t *config_types)
{
    tx_packet(LINK_REQ_CONF_CMD,config_size,config_types);
    return;
}

//Send 'fresca-link' packet
void tx_packet(uint8_t cmd_type, uint8_t payload_len, uint8_t *payload)
{
    uint8_t checksum;
    
    Serial1.write(LINK_START_CMD);
    Serial1.write(cmd_type);
    Serial1.write(payload_len);
    checksum = 0xff; //Init checksum
    checksum = calc_checksum(payload_len,checksum);
    for (uint8_t i=0; i < payload_len; i++)
    {
      Serial1.write(payload[i]);
      checksum = calc_checksum(payload[i],checksum);
    }
    Serial1.write(checksum);
    
    return;
}

//Receive 'fresca-link' packet, if return value is true, it indicates correctly received packet
//
//Macro for doing an arduino read with timeout
#define SERIAL1_READ(VAR,TIM_VAR,INIT) TIM_VAR=INIT; \
                                       while(!Serial.available()){  \
                                       if (!TIM_VAR){return false;} \
                                       TIM_VAR--;}; \
                                       VAR=Serial1.read()
uint8_t rx_packet(uint8_t *type, uint8_t *len, uint8_t *buffer, uint8_t max_len)
{
    uint8_t recv, timeout;
    if (Serial1.available())
    {
        recv=Serial1.read();
        if (recv==LINK_START_CMD)
        {
            //Got a packet coming
            SERIAL1_READ(recv,timeout,-1); //Type of command
            *type=recv;
            
            uint8_t checksum, payload_len;
            SERIAL1_READ(payload_len,timeout,-1); //Read Payload length
            *len=payload_len; //Write payload length
            if (payload_len > max_len || payload_len == 0)
            {
                //Invalid length
                return false;
            }
            checksum = 0xff; //Init checksum
            checksum = calc_checksum(payload_len,checksum);
            
            for (uint8_t i=0;i<payload_len;i++)
            {
                SERIAL1_READ(recv,timeout,-1); //Read Payload
                buffer[i]=recv;     //Write payload
                checksum = calc_checksum(recv,checksum);
            }

            SERIAL1_READ(recv,timeout,-1); //Receive checksum
            if (recv != checksum)
            {
                //Wrong checksum
                return false;
            }
            return true; //Correctly received a packet
        }
    }
    return false; //No packet incoming
}

//Calculate checksum
uint8_t calc_checksum(uint8_t data, uint8_t checksum)
{
    return checksum + data; //Do simple checksum
}

//Calculate 8-bit CRC
//Polynomial is taken from Koopman, P. & Chakravarty, T., "Cyclic Redundancy Code (CRC) Polynomial Selection for Embedded Networks,“ DSN04, June 2004
// #define CRC_POLY 0xA6
// uint8_t calc_crc(uint8_t data, uint8_t crc)
// {
    // for (uint8_t i = 0; i<8; i++)
    // {
        // uint8_t result = (crc ^ data) & 0x01;
        // crc >>= 1;
        // if (result)
        // {
            // crc ^= CRC_POLY;
        // }
        // data >>= 1;
    // }
    // return crc;
// }