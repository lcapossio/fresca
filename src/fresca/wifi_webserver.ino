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

#include "WiFiEsp.h" //Actual WiFi driver library
#include "wifi_webserver.h"
#include "wifi_ap.h" //Where the SSID and the password are set

#include "fresca.h"



// Emulate Serial1 on pins RX19,TX18 if not present
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




















static char * g_currSSID = ssid;
static char g_currIPADDR[16];
static int  g_status = WL_NO_SHIELD;     // the Wifi radio's status
static int  reqCount = 0;                // number of requests received

WiFiEspServer g_server(80);


uint8_t setup_webserver()
{
    // initialize serial for debugging
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
    // while (Serial1.available()) Serial1.read();
    // Serial1.flush();         // wait for send buffer to empty
    // initialize serial for ESP module
    Serial1.begin(9600);
    
    //Test some commands
    sendATcommand("AT+GMR","OK",3000);
    sendATcommand("AT+UART_CUR?","OK",3000);
    // sendATcommand("AT+UART_DEF=9600,8,1,0,0","",3000);
    
    // initialize ESP module
    WiFi.init(&Serial1);

    // check for the presence of the shield
    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("***ERROR: WiFi shield not present");
        return false;
    }

    uint8_t retries=0;
    // attempt to connect to WiFi network
    while ( (g_status != WL_CONNECTED) && (retries < 3) ) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network
        g_status = WiFi.begin(ssid, pass);
        retries++;
    }
    
    if (g_status != WL_CONNECTED)
    {
        Serial.println("***ERROR: Failed connecting to the network!!!");
        return false;
    }
    
    //Connnected and happy
    Serial.println("You're connected to the network");
    printWifiStatus();
    
    IPAddress ip;
    ip=WiFi.localIP();
    snprintf(g_currIPADDR,16,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);

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
        
        //Check if it is an HTTP GET request, otherwise just quit
        client.setTimeout(500);
        if (client.find("GET"))
        {
            if (client.find("HTTP"))
            {
                send_response=true; //We received HTTP GET request
            }
            else
            {
                Serial.println("***Bad GET request*");
            }
        }
        else
        {
            Serial.println("***Not a GET request*");
        }
        
        if (send_response)
        {
            client.setTimeout(3000);
            client.find("\r\n\r\n"); //Wait for HTTP GET request to finish (with a blank line)
            
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

        client.flush(); //Flush any remaining data
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

inline int GetStatus()
{
    return g_status;
}

inline char* GetSSID()
{
    return g_currSSID;
}

inline char* GetIPAddr()
{
    return g_currIPADDR;
}