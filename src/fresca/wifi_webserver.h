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

#ifndef FRESCA_WIFI_WEBSERVER_H
#define FRESCA_WIFI_WEBSERVER_H
    
    #define HAVE_HWSERIAL1  // Emulate Serial1 on pins RX19,TX18 if Hardware serial is not present
    
    ////////////////////////////////////////
    //Function declarations
    uint8_t setup_webserver();
    void run_webserver(TEMP_DATA_TYPE *temp);
    void printWifiStatus();
    inline char* GetSSID();
    inline char* GetIPAddr();
    ////////////////////////////////////////
    
#endif