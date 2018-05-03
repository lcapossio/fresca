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

#ifndef FRESCA_LINK_H
#define FRESCA_LINK_H

  #include <Arduino.h>

  void setup_fresca_link();
  void send_temp_link(uint8_t payload_len, uint8_t *payload);
  void send_request_config(uint8_t config_size,uint8_t config_types);
  void tx_packet(uint8_t type, uint8_t payload_len, uint8_t *payload);
  uint8_t calc_checksum(uint8_t data, uint8_t checksum);
  
  //Start of packet and frame signalling
  #define LINK_START_CMD      0xA5
  #define LINK_FLUSH_CMD      0x1A
  //Types of commands
  #define LINK_IDLE_CMD       0xAA
  #define LINK_TEMP_CMD       0xAB
  #define LINK_HUM_CMD        0xAC
  #define LINK_REQ_CONF_CMD   0xAD
  #define LINK_REG_RD_CMD     0xAE
  #define LINK_REG_WR_CMD     0xAF
  //Acknowledges from packets
  #define LINK_OK_CMD         0x05
  #define LINK_NOK_CMD        0xFA

#endif