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

#ifndef FRESCA_SPI_H
#define FRESCA_SPI_H

    #include <Arduino.h>
    #include <fresca.h>
    #include <SPI.h>

    void setup_spi_slave();
    // void close_spi_comms();
    // void begin_spi_comms();
    // uint8_t is_spi_in_process();
    void spi_process(uint8_t payload_length, uint8_t *payload);
    uint8_t calc_crc(uint8_t data, uint8_t crc);

    #define SPI_START_CMD 0xA5
    #define SPI_FLUSH_CMD 0x1A
    #define SPI_IDLE_CMD  0xAA
    #define SPI_TEMP_CMD  0xAB
    #define SPI_HUM_CMD   0xAC
    #define SPI_OK_CMD    0x05
    #define SPI_NOK_CMD   0xFA

#endif