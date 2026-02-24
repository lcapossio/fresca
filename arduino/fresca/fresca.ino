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

#include <LiquidCrystal.h>
#include <TM1637Display.h>
#include <DFR_Key.h>
#include <EEPROM.h>
#include <TempController.h>
#include <fresca_link.h>
#include <fresca_pinout.h>
#include <fresca_sensor.h>
#include <fresca_utils.h>
#include <fresca.h>

#include "fresca_globals.h"
#include "fresca_ui.h"
#include "fresca_control.h"

////////////////////////////////////////
// Global variables definitions
// These are declared as extern in fresca_globals.h
uint8_t         g_showSensLCD = 0;                  
uint8_t         g_showTempHumLCD[NUM_SENSORS]= {0}; 
TEMP_DATA_TYPE  g_TempReading[NUM_SENSORS]   = {0}; 
HUM_DATA_TYPE   g_HumReading[NUM_SENSORS]    = {0}; 
TEMP_DATA_TYPE  g_CoolOnThresh[NUM_SENSORS]  = {0};
TEMP_DATA_TYPE  g_CoolOffThresh[NUM_SENSORS] = {0};
TEMP_DATA_TYPE  g_HeatOnThresh[NUM_SENSORS]  = {0};
TEMP_DATA_TYPE  g_HeatOffThresh[NUM_SENSORS] = {0};
TEMP_DATA_TYPE  g_OffsetSensor[NUM_SENSORS]  = {0};

TM1637Display                  * g_disp7seg[NUM_SENSORS];      
fresca_sensor                  * g_fresca_sensor;              
LiquidCrystal                    lcd(gc_lcd_pins[0], gc_lcd_pins[1], gc_lcd_pins[2], gc_lcd_pins[3], gc_lcd_pins[4], gc_lcd_pins[5]); 
DFR_Key                          keypad(gc_keypad_pins);       
TempController<TEMP_DATA_TYPE> * TempControllers[NUM_SENSORS]; 
////////////////////////////////////////

void setup(void)
{
    uint8_t i;
    noInterrupts();
    
    Serial.begin(9600);
    Serial.println("---------------");
    Serial.println("'fresca' project");
    Serial.println("Aguantia  ...   ");
    Serial.println("---------------");

    setup_fresca_link();
    
    Serial.print("Initializing LCD...");
    lcd.begin(LCD_WIDTH, LCD_HEIGHT, 1);
    lcd.setCursor(0,0);
    lcd.print("'fresca' project");
    lcd.setCursor(0,1);
    lcd.print("Aguantia  ...   ");
    Serial.println("Done!");

    Serial.print("Initializing 7-segment displays...");
    for (i = 0; i < NUM_7SEG; i++)
    {
        g_disp7seg[i] = new TM1637Display(gc_7seg_clk_pins, gc_7seg_dio_pins[i]);
        g_disp7seg[i]->setBrightness(0x0f, true);
        g_disp7seg[i]->showNumberDec(1305, true);
    }
    delay_noInterrupts(400);
    for (i = 0; i < NUM_7SEG; i++)
    {
        g_disp7seg[i]->setBrightness(0x00, false);
        g_disp7seg[i]->showNumberDec(1305, true);
    }
    delay_noInterrupts(400);
    for (i = 0; i < NUM_7SEG; i++)
    {
        g_disp7seg[i]->setBrightness(0x0f, true);
        g_disp7seg[i]->showNumberDec(1305, true);
    }
    Serial.println("Done!");

    Serial.print("Initializing temperature sensors...");
    g_fresca_sensor = new fresca_sensor(NUM_SENSORS, gc_temp_sens_type, gc_temp_sens_pins, &Serial);
    Serial.println("Done!");
    
    Serial.print("Initializing keypad...");
    keypad.setRate(KEYPAD_REFRESH_RATE);
    Serial.println("Done!");
    
    g_showSensLCD = 0; 
    
    uint16_t eeprom_offset;
    Serial.print("Trying to recall EEPROM data ... ");
    if ((EEPROM.read(EEPROM_MAGIC_NUM_ADDR) != EEPROM_MAGIC_NUM_VALUE) || (SelectKeyPressed(&keypad)))
    {
        if (EEPROM.read(EEPROM_MAGIC_NUM_ADDR) != EEPROM_MAGIC_NUM_VALUE)
        {
            Serial.print("***NOT found magic number, writing default values ...");
            EEPROM.put(EEPROM_MAGIC_NUM_ADDR, (uint8_t) EEPROM_MAGIC_NUM_VALUE);
        }
        else
        {
            Serial.print("***SELECT key pressed, writing default values ...");
        }
        
        eeprom_offset = EEPROM_START_ADDR;
        for (i=0; i<NUM_SENSORS; i++)
        {
            g_CoolOnThresh[i] = COOLON_DFLT;
            EEPROM.put(eeprom_offset, g_CoolOnThresh[i]); eeprom_offset += sizeof(TEMP_DATA_TYPE);
            g_CoolOffThresh[i] = COOLOFF_DFLT;
            EEPROM.put(eeprom_offset, g_CoolOffThresh[i]); eeprom_offset += sizeof(TEMP_DATA_TYPE);
            g_HeatOnThresh[i] = HEATON_DFLT;
            EEPROM.put(eeprom_offset, g_HeatOnThresh[i]); eeprom_offset += sizeof(TEMP_DATA_TYPE);
            g_HeatOffThresh[i] = HEATOFF_DFLT;
            EEPROM.put(eeprom_offset, g_HeatOffThresh[i]); eeprom_offset += sizeof(TEMP_DATA_TYPE);
        }
    }
    else
    {
        Serial.print("Successfully found magic number, recalling data ...");
        eeprom_offset = EEPROM_START_ADDR;
        for (i=0; i<NUM_SENSORS; i++)
        {
            EEPROM.get(eeprom_offset, g_CoolOnThresh[i]); eeprom_offset += sizeof(TEMP_DATA_TYPE);
            EEPROM.get(eeprom_offset, g_CoolOffThresh[i]); eeprom_offset += sizeof(TEMP_DATA_TYPE);
            EEPROM.get(eeprom_offset, g_HeatOnThresh[i]); eeprom_offset += sizeof(TEMP_DATA_TYPE);
            EEPROM.get(eeprom_offset, g_HeatOffThresh[i]); eeprom_offset += sizeof(TEMP_DATA_TYPE);
        }
    }
    Serial.println("Done!");
    
    Serial.print("Initializing temperature controllers (relays)...");
    uint8_t pins[2];
    TEMP_DATA_TYPE limits[4];
    TEMP_DATA_TYPE thresholds[4];
    for (i=0; i<NUM_SENSORS; i++)
    {
        thresholds[0]=g_CoolOnThresh[i]; thresholds[1]=g_CoolOffThresh[i];
        thresholds[2]=g_HeatOnThresh[i]; thresholds[3]=g_HeatOffThresh[i];
        pins[0]=g_CoolerPins[i]; pins[1]=g_HeaterPins[i];
        limits[0]=MAX_TEMP; limits[1]=MIN_TEMP;
        limits[2]=MAX_TEMP; limits[3]=MIN_TEMP;
        if (g_CoolerEn[i] && g_HeaterEn[i])
        {
            TempControllers[i] = new TempController<TEMP_DATA_TYPE>(TempController_type::Both, pins, thresholds, limits, THRESHOLD_STEP);
        }
        else if (g_CoolerEn[i])
        {
            TempControllers[i] = new TempController<TEMP_DATA_TYPE>(TempController_type::Cool, pins, thresholds, limits, THRESHOLD_STEP);
        }
        else if (g_HeaterEn[i])
        {
            TempControllers[i] = new TempController<TEMP_DATA_TYPE>(TempController_type::Heat, &pins[1], &thresholds[2], &limits[2], THRESHOLD_STEP);
        }
    }
    if (g_WaterPumpEn[0] != 0)
    {
      pinMode(g_WaterPump[0], OUTPUT);
      digitalWrite(g_WaterPump[0], RELAY_OFF);
    }
    Serial.println("Done!");
    
    //////////////////////////////////////////////////
    // Initialize Timer1 interrupt
    Serial.print("Initializing timer1 ...");
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    TCCR1B |= (1 << WGM12) | (1 << CS12);               
    Serial.println("Done!");
    
    delay_noInterrupts(INIT_DELAY);
}

// Timer1 compare interrupt service routine
ISR(TIMER1_COMPA_vect)
{
    main_menu();
    TCNT1  = 0; 
}

void loop(void)
{
    lcd.clear();                
    interrupts();               
    SET_TIMER1(TIMER_20MS);     
    TIMSK1 |= (1 << OCIE1A);    
    
    Serial.println("***Executing Main Loop...");
    uint32_t start_time = millis();
    read_temp_sensors();
    
    while(true)
    {
        if (millis() - start_time >= (TEMP_POLL_MSEC + 10))
        {
            start_time = millis();
            read_temp_sensors();
            send_temp_link(NUM_SENSORS * 2, (uint8_t *) g_TempReading);
        }
    }
}