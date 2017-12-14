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

#include "fresca_spi.h"


enum spi_state_type {st_idle,st_command,st_tx_payload_length,st_tx_payload,st_crc};
static spi_state_type g_spi_state;

void setup_spi_slave()
{
    // Turn on SPI in slave mode
    SPCR |= bit (SPE);

    // Slave Output/Master Input, set as OUTPUT
    pinMode(MISO, OUTPUT);

    // now turn on interrupts
    SPI.attachInterrupt();
    
    SPDR = SPI_NOK_CMD; //default answer
    g_spi_state = st_idle;
}

//Indicate we're processing data and we're unable to keep processing SPI transactions
void close_spi_comms()
{
    SPDR = SPI_NOK_CMD; //default answer
    SPCR &= ~SPIE; //disable spi interrupt
    g_spi_state = st_idle;
}

//Resume SPI
void begin_spi_comms()
{
    //Re-enable the interrupt
    SPCR |= SPIE; //enable spi interrupt
}

uint8_t is_spi_in_process()
{
    if (g_spi_state!=st_idle)
    {
        return true;
    }
    return false;
}

//Process SPI communication as slave
//Payload has to be [byte0,byte1,byte2 ... byteN]
void spi_process(uint8_t payload_length, uint8_t *payload)
{
    uint8_t recv_byte = SPDR;  // grab byte from SPI Data Register
    static uint8_t crc;
    static uint8_t payload_idx;

    // if (recv_byte == SPI_FLUSH_CMD)
    // {
        // SPDR = SPI_OK_CMD; //OK flush, go to idle state
        // g_spi_state=st_idle;
        // return;
    // }
    
    //Main SPI state machine code
    switch (g_spi_state)
    {
        default:
        case st_idle:
            if (recv_byte == SPI_START_CMD)
            {
                SPDR = SPI_OK_CMD; //Wait for command
                g_spi_state=st_command;
            }
            else
            {
                //Bad start
                SPDR = SPI_NOK_CMD; //Wait to synch communication
            }
        break;
        case st_command:
            if (recv_byte == SPI_TEMP_CMD)
            {
                //Recognized command, return temperature readings
                SPDR = SPI_OK_CMD; //Start transferring
                g_spi_state=st_tx_payload_length;
                payload_idx = 0;
                crc = 0xff; //Init CRC
            }
            else
            {
                //ERROR, return to idle
                SPDR = SPI_NOK_CMD; //Unrecognized command
                g_spi_state=st_idle;
            }
        break;
        case st_tx_payload_length:
            if (recv_byte != SPI_OK_CMD)
            {
                //ERROR, return to idle
                SPDR = SPI_NOK_CMD;
                g_spi_state=st_idle;
            }
            else
            {
                //
                //Transfer payload_length
                SPDR = payload_length;
                crc  = calc_crc(payload_length,crc);
                g_spi_state=st_tx_payload;
            }
        break;
        case st_tx_payload:
            if (recv_byte != SPI_OK_CMD)
            {
                //ERROR, return to idle
                SPDR = SPI_NOK_CMD;
                g_spi_state=st_idle;
            }
            else
            {
                //
                //Transfer next byte
                SPDR = payload[payload_idx];
                crc  = calc_crc(payload[payload_idx],crc);
                payload_idx++; //Increase index
                //
                if (payload_idx == payload_length)
                {
                    //Finished go to send CRC
                    g_spi_state=st_crc;
                }
            }
        break;
        case st_crc:
            if (recv_byte != SPI_OK_CMD)
            {
                //ERROR, return to idle
                SPDR = SPI_NOK_CMD;
                g_spi_state=st_idle;
            }
            else
            {
                //Transfer CRC
                SPDR = crc;
                g_spi_state=st_idle;
            }
        break;
    }
}

//Calculate 8-bit CRC
//Polynomial is taken from Koopman, P. & Chakravarty, T., "Cyclic Redundancy Code (CRC) Polynomial Selection for Embedded Networks,“ DSN04, June 2004
#define CRC_POLY 0xA6
uint8_t calc_crc(uint8_t data, uint8_t crc)
{
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
    return crc + data;
}