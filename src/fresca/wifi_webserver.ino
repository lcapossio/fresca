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
            Modified from the "WiFiEsp example: WebServer"
            
            
            
            
            
            
            
            
            
            
            
            
            
*********************************************************************************************
*/

#include "WiFiEsp.h"
#include "wifi_webserver.h"
#include "wifi_ap.h"

#include "fresca.h"
#include "utils.h"


#define HAVE_HWSERIAL1

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(19, 18); // RX, TX
#endif



#define AT_COMMAND_RESPONSE_BUF_SIZE 64

uint8_t GetAnswer(char* expected_answer, uint16_t timeout)
{
    uint8_t x;
    uint8_t temp_char;
    uint8_t answer;
    char response[AT_COMMAND_RESPONSE_BUF_SIZE]="";
    uint32_t start_time;
    
    while( Serial1.available() > 0) Serial1.read();    // Wait for clean input buffer
    delay(20); // Delay to be sure no passed commands interfere
    
    // this loop waits for the answer
    start_time = millis();
    x=0;
    answer=0;
    do
    {
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(Serial1.available() != 0){
            temp_char = Serial1.read();
            Serial.write(temp_char);
            if (isspace(temp_char) || (x == AT_COMMAND_RESPONSE_BUF_SIZE))
            {
                x=0;
                // check if the desired answer is in the response of the module
                if (strstr(response, expected_answer) != NULL)
                {
                    answer = 1;
                }
            }
            else
            {
                response[x] = temp_char;
                x++;
            }
        }
    // Waits for the answer with time out
    } while((answer == 0) && ((millis() - start_time) < timeout));
    
    return answer;
}

uint8_t sendATcommand(char* ATcommand, char* expected_answer, uint16_t timeout)
{
    uint8_t answer;
    
    Serial1.println(ATcommand);    // Send the AT command 
    
    if (expected_answer == "")
    {
        //Not expecting an answer return immediately
        Serial.print(ATcommand);
        Serial.println(" command successful");
        Serial1.flush();
        return 1;
    }
    
    
    Serial.println("-------");
    
    // while( Serial1.available() > 0) Serial1.read();    // Wait for clean input buffer

    // delay(20); // Delay to be sure no passed commands interfere

    answer=GetAnswer(expected_answer,timeout);
    
    Serial.println();
    Serial.print(ATcommand);
    if (answer!=0)
    {
        Serial.println(" command successful");
    }
    else
    {
        Serial.println(" command FAILED");
    }
    Serial.println("-------");
    Serial1.flush();
    return answer;
}




















int status = WL_IDLE_STATUS;     // the Wifi radio's status
int reqCount = 0;                // number of requests received

WiFiEspServer g_server(80);


uint8_t setup_webserver()
{
    // initialize serial for debugging
    // initialize serial for ESP module
    // Serial1.begin(115200);
    // sendATcommand("AT+RST","ready",3000);
    // sendATcommand("AT+GMR","OK",3000);
    // sendATcommand("AT+UART_CUR?","OK",3000);
    
    // sendATcommand("AT+UART_CUR=9600,8,1,0,0","",3000);
    // Serial1.flush();         // wait for send buffer to empty
    // delayMicroseconds(5000); // let last character be sent
    // while (Serial1.available()) Serial1.read();
    // Serial1.flush();         // wait for send buffer to empty
    // Serial1.end();           // close serial
    Serial1.begin(9600);
    // while (Serial1.available()) Serial1.read();
    // Serial1.flush();         // wait for send buffer to empty
    
    sendATcommand("AT+GMR","OK",3000);
    sendATcommand("AT+UART_CUR?","OK",3000);
    // sendATcommand("AT+GMR","OK",3000);
    // sendATcommand("AT+UART_DEF=9600,8,1,0,0","",3000);
    
    // initialize ESP module
    WiFi.init(&Serial1);
    
    // sendATcommand("AT+GMR","OK",3000);
    // sendATcommand("AT+UART_CUR?","OK",3000);

    // check for the presence of the shield
    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("***ERROR: WiFi shield not present");
        return false;
    }

    // attempt to connect to WiFi network
    while ( status != WL_CONNECTED) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network
        status = WiFi.begin(ssid, pass);
    }
    
    Serial.println("You're connected to the network");
    printWifiStatus();

    // start the web server on port 80
    g_server.begin();
    // g_server.setNoDelay(true);
    
    //Wifi up and running
    return true;
}


void run_webserver(TEMP_DATA_TYPE *temp, uint8_t *sensor_present)//, Cool)
{
    //Serial.println("Searching for clients...");
    char    print_buf0[LCD_WIDTH+1];
    char    print_buf1[64];
    char    send_buf[512]; send_buf[0]='\0';
    char    c;
    uint8_t i,send_response;
    uint32_t start;
    boolean currentLineIsBlank = true;
    
    // listen for incoming clients
    WiFiEspClient client = g_server.available();
    
    if (client)
    {
        // Serial.println("New client");
        start = millis(); //Start measuring time

        send_response=false;
        
        //Check if it is an HTTP get request, otherwise just quit
        client.setTimeout(200);
        if (client.find("GET"))
        {
            send_response=true;
        }
        
        if (send_response)
        {
            
            Serial.println("*Sending response");
            // if you've gotten to the end of the line (received a newline
            // character) and the line is blank, the http request has ended,
            // so you can send a reply
            // send a standard http response header
            // use \r\n instead of many println statements to speedup data send
            // client.print(
            // strcat(send_buf,("HTTP/1.1 200 OK\r\n"
            strcat(send_buf,("HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Refresh: 10\r\n"         // refresh the page automatically every 10 sec
            "Connection: close\r\n"  // the connection will be closed after completion of the response
            "\r\n"
            "<!DOCTYPE HTML>\r\n"
            "<html>\r\n"
            "<h1>fresca project!</h1>\r\n"
            "<br>\r\n"
            ));
            // client.print("Requests received: ");
            // client.print(++reqCount);
            
            //Send temperature
            for (i=0; i<NUM_DS1820_SENSORS; i++)
            {
                PrintTempBuf(temp[i],sensor_present[i]==false,print_buf0,0);
                sprintf(print_buf1,"Sensor %d: %s<br>\r\n",i,print_buf0);
                strcat(send_buf,print_buf1);
                // Serial.print(print_buf1);
            }
            //Close webpage
            strcat(send_buf,("<br>\r\n"
                            "<i>Don't poll faster than 10 sec</i><br>\r\n"
                            "</html>\r\n"));
            //Send all the stuff in a big chunk, it is way faster than multiple print calls
            client.print(send_buf);
            Serial.println("*Sent webpage data");
        }
        
        // close the connection:

        client.flush();
        client.stop();

        Serial.println("*Client disconnected");
        //Measure time
        sprintf(print_buf1,"Webserver time: %ld\n", millis() - start);
        Serial.print(print_buf1);
    }
}


void printWifiStatus()
{
    // print the SSID of the network you're attached to
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
}