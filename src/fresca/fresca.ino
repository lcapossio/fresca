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

*This program will correctly display temperature on the LCD and 7-seg displays for up to 8 sensors
*It will also monitor all the sensors temperature and activate cooling according to user settings
*Pinout can be lookedup/changed in this file where it says " PINOUT_LINE "
*The menu can be navigated by pressing the 'Select' key, which will cycle through:
 temperature display, CoolOn threshold setting, CoolOff threshold setting and Offset calibration setting
*The CoolOn and CoolOff settings can be modified by pressing the right/left keys,
 which will in turn activate the relay when the temperature falls below CoolOn,
 and will turn the relay off when the temperature rises above CoolOff
*Current sensor selected can be changed in main menu by pressing up/down keys
*Stores Cooling thresholds and other data in EEPROM
*To know if there is valid data in the EEPROM (e.g. arduino was used for other projects)
 the code will search for a magic number '0x5A' in the EEPROM at address 0, if it doesn't find it
 it will write the magic number, and the default values for the thresholds
*To recover the EEPROM default state, keep SELECT key pressed at boot up until the main menu is reached
*Temperature display is defaulted to degrees celsius, but can be changed to fahrenheit with TEMP_FAHRENHEIT
 Celsius temperature display on 4-digit 7-segment LCD is two leftmost digits are whole part and two rightmost are fractional part
 Fahrenheit temperature display on 4-digit 7-segment LCD is three leftmost digits are whole part and the rightmost is fractional part
 All temperatures are represented internally with 16 bits signed 'Q11.4' fixed point format (celsius)

*********************************************************************************************
*/

#include <LiquidCrystal.h>
#include <TM1637Display.h>
#include <DFR_Key.h>
#include <EEPROM.h>
#include "ds1820.h"
#include "utils.h"
#include "fresca.h"

////////////////////////////////////////
// ****** DEFINE PINOUT HERE ****** PINOUT_LINE
//Define pinout for MAXIMUM number of sensors (MAX_NUM_DS1820_SENSORS), if less sensors are actually implemented (NUM_DS1820_SENSORS), the rest of the pins will not be used or touched at all
const uint8_t gc_7seg_dio_pins[MAX_NUM_DS1820_SENSORS]   = {7,14,15,16,17,18,19,20};   //All TM1637 DIO are specified here
const uint8_t gc_7seg_clk_pins                           = {8};                        //One clock for all TM1637 displays
const uint8_t g_CoolSwitch[MAX_NUM_DS1820_SENSORS]       = {9,21,22,23,24,25,26,27};   //Contains pin number for relay index
const uint8_t g_HeatSwitch[MAX_NUM_DS1820_SENSORS]       = {35,36,37,38,39,40,41,42};  //Contains pin number for relay index
const uint8_t gc_ds1820_pins[MAX_NUM_DS1820_SENSORS]     = {6,28,29,30,31,32,33,34};   //DS18B20 Digital temperature sensor
const uint8_t gc_lcd_pins[6]                             = {12, 11, 5, 4, 3, 2};       //LCD 16x2 based on the Hitachi HD44780 controller (rs, enable, d4, d5, d6, d7)
const uint8_t gc_keypad_pins                             = 0;                          //Analog Keypad on the LCD PCB (has to be an analog pin)
////////////////////////////////////////

////////////////////////////////////////
//Global variables
uint8_t  g_showtempLCD = 0;                         //Set to the Sensor number you want to display on the LCD (0: sensor0)
TEMP_DATA_TYPE  g_TempReading[NUM_DS1820_SENSORS]   = {0}; //Last successful temperature reading
TEMP_DATA_TYPE  g_CoolOnThresh[NUM_DS1820_SENSORS]  = {0};
TEMP_DATA_TYPE  g_CoolOffThresh[NUM_DS1820_SENSORS] = {0};
TEMP_DATA_TYPE  g_HeatOnThresh[NUM_DS1820_SENSORS]  = {0};
TEMP_DATA_TYPE  g_HeatOffThresh[NUM_DS1820_SENSORS] = {0};
TEMP_DATA_TYPE  g_OffsetSensor[NUM_DS1820_SENSORS]  = {0};
////////////////////////////////////////

////////////////////////////////////////
//Define objects
TM1637Display * g_disp7seg[NUM_DS1820_SENSORS];     //7segment displays
DS1820 *        g_ds1820[NUM_DS1820_SENSORS];       //DS18B20 Digital temperature sensor
LiquidCrystal   lcd(gc_lcd_pins[0], gc_lcd_pins[1], gc_lcd_pins[2], gc_lcd_pins[3], gc_lcd_pins[4], gc_lcd_pins[5]); //(rs, enable, d4, d5, d6, d7) //LCD 16x2 based on the Hitachi HD44780 controller
DFR_Key         keypad(gc_keypad_pins);             //Analog Keypad on the LCD
////////////////////////////////////////

////////////////////////////////////////
//Start main code
////////////////////////////////////////

void loop(void)
{
    lcd.clear();                //Wipe the screen
    interrupts();               // enable all interrupts
    SET_TIMER1(TIMER_20MS);     // Set timer
    TIMSK1 |= (1 << OCIE1A);    // enable timer compare interrupt
    
    Serial.print("***Executing Main Loop...");Serial.println();
    
    //MAIN Infinite loop
    MAIN_LOOP:while(true)
    {
        read_temp_sensors();
        delay_noInterrupts(TEMP_POLL_SEC); //750ms conversion time for 12 bits
    }
}

//Setup function run after POR
void setup(void)
{
    uint8_t i;
    
    // Disable all interrupts
    noInterrupts();
    
    //////////////////////////////////////////////////
    //Initialize Serial
    Serial.begin(9600);
    Serial.print("---------------");Serial.println();
    Serial.print("'fresca' project");Serial.println();
    Serial.print("Aguantiaaaaa ...");Serial.println();
    Serial.print("---------------");Serial.println();
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
    //Initialize LCD
    Serial.print("Initializing LCD...");
    lcd.begin(LCD_WIDTH, LCD_HEIGHT,1);
    lcd.setCursor(0,0);
    lcd.print("'fresca' project");
    lcd.setCursor(0,1);
    lcd.print("Aguantiaaaaa ...");
    Serial.print("Done!");Serial.println();
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
    //Initialize Displays
    Serial.print("Initializing 7-segment displays...");
    for (i = 0; i < NUM_DS1820_SENSORS; i++)
    {
        //Create object
        g_disp7seg[i] = new TM1637Display(gc_7seg_clk_pins,gc_7seg_dio_pins[i]);
        //
        g_disp7seg[i]->setBrightness(0x0f,true); //Turn-on
        g_disp7seg[i]->showNumberDec(1305,true);
    }
    //Blink them
    delay_noInterrupts(400);
    for (i = 0; i < NUM_DS1820_SENSORS; i++)
    {
        g_disp7seg[i]->setBrightness(0x00,false);//Turn-off
        g_disp7seg[i]->showNumberDec(1305,true); //Write the number again to turn off
    }
    delay_noInterrupts(400);
    for (i = 0; i < NUM_DS1820_SENSORS; i++)
    {
        g_disp7seg[i]->setBrightness(0x0f,true); //Turn-on again (end blinking)
        g_disp7seg[i]->showNumberDec(1305,true);
    }
    Serial.print("Done!");Serial.println();
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
    //Initialize temperature sensors
    Serial.print("Initializing DS18B20 temperature sensors...");
    for (i=0;i<NUM_DS1820_SENSORS;i++)
    {
        //Create object
        g_ds1820[i] = new DS1820(gc_ds1820_pins[i],&Serial);
        //Default is 12-bit resolution
        g_ds1820[i]->StartTemp(); //Start temperature conversion
    }
    Serial.print("Done!");Serial.println();
    //////////////////////////////////////////////////
    
    //////////////////////////////////////////////////
    //Initialize keypad
    Serial.print("Initializing keypad...");
    keypad.setRate(KEYPAD_REFRESH_RATE); //Sets the sample rate at once every x milliseconds.
    Serial.print("Done!");Serial.println();
    //////////////////////////////////////////////////
    
    //////////////////////////////////////////////////
    //Initialize Relays
    Serial.print("Initializing relays...");
    for (i=0;i<NUM_DS1820_SENSORS;i++)
    {
        pinMode(g_CoolSwitch[i], OUTPUT);
        digitalWrite(g_CoolSwitch[i], (RELAY_ACTIVE!=0) ? LOW : HIGH );
    }
    Serial.print("Done!");Serial.println();
    //////////////////////////////////////////////////
    
    g_showtempLCD=0; //Show temperature for sensor0
    
    //////////////////////////////////////////////////
    //Recall EEPROM values
    uint16_t eeprom_offset;
    Serial.print("Trying to recall EEPROM data ... ");
    if ( (EEPROM.read(EEPROM_MAGIC_NUM_ADDR) != EEPROM_MAGIC_NUM_VALUE) || (SelectKeyPressed(&keypad)) )
    {
        //We haven't written this EEPROM before, or user is indicating a reset of the EEPROM
        //Store new default values
        if (EEPROM.read(EEPROM_MAGIC_NUM_ADDR) != EEPROM_MAGIC_NUM_VALUE)
        {
            Serial.print("***NOT found magic number, writing default values ...");
            EEPROM.put(EEPROM_MAGIC_NUM_ADDR, (uint8_t) EEPROM_MAGIC_NUM_VALUE); //Write magic number
        }
        else
        {
            Serial.print("***SELECT key pressed, writing default values ...");
        }
        
        //Write default values
        eeprom_offset = EEPROM_START_ADDR;
        for (i=0; i<NUM_DS1820_SENSORS; i++)
        {
            g_CoolOnThresh[i] = COOLON_DFLT;
            EEPROM.put(eeprom_offset, g_CoolOnThresh[i]); eeprom_offset+=sizeof(TEMP_DATA_TYPE);
            g_CoolOffThresh[i] = COOLOFF_DFLT;
            EEPROM.put(eeprom_offset, g_CoolOffThresh[i]);eeprom_offset+=sizeof(TEMP_DATA_TYPE);
            g_HeatOnThresh[i] = HEATON_DFLT;
            EEPROM.put(eeprom_offset, g_HeatOnThresh[i]); eeprom_offset+=sizeof(TEMP_DATA_TYPE);
            g_HeatOffThresh[i] = HEATOFF_DFLT;
            EEPROM.put(eeprom_offset, g_HeatOffThresh[i]);eeprom_offset+=sizeof(TEMP_DATA_TYPE);
        }
    }
    else
    {
        //We have written this EEPROM before, recall the data
        Serial.print("Successfully found magic number, recalling data ...");
        eeprom_offset = EEPROM_START_ADDR;
        for (i=0; i<NUM_DS1820_SENSORS; i++)
        {
            EEPROM.get(eeprom_offset, g_CoolOnThresh[i]); eeprom_offset+=sizeof(TEMP_DATA_TYPE);
            EEPROM.get(eeprom_offset, g_CoolOffThresh[i]);eeprom_offset+=sizeof(TEMP_DATA_TYPE);
            EEPROM.get(eeprom_offset, g_HeatOnThresh[i]); eeprom_offset+=sizeof(TEMP_DATA_TYPE);
            EEPROM.get(eeprom_offset, g_HeatOffThresh[i]);eeprom_offset+=sizeof(TEMP_DATA_TYPE);
        }
    }
    Serial.print("Done!");Serial.println();
    //////////////////////////////////////////////////
    
    //////////////////////////////////////////////////
    // Initialize Timer1 interrupt
    Serial.print("Initializing timer1 ...");
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0; //Timer count register
    TCCR1B |= (1 << WGM12) | (1 << CS12);               // Clear Timer on Compare (CTC) mode ; 256 prescaler
    Serial.print("Done!");Serial.println();
    //////////////////////////////////////////////////
    
    Serial.print("***************************");Serial.println();
    Serial.print("***Starting main program***");Serial.println();
    Serial.print("***************************");Serial.println();
    
    delay_noInterrupts(INIT_DELAY);
}

//////////////////////////////////////////
// Timer1 compare interrupt service routine
//////////////////////////////////////////
ISR(TIMER1_COMPA_vect)
{
    //Run main menu
    main_menu();
    //Reset TIMER1 count
    TCNT1  = 0; //Timer count register
}

//Main menu code
//Poll keys, main state machine
inline void main_menu()
{
    enum state_type {st_show_temp,st_change_CoolOn,st_change_CoolOff,st_change_HeatOn,st_change_HeatOff,st_calib_sensor}; //State machine states
    char print_buf[MAX_BUF_CHARS];
    DFR_Key_type tempKey            = NO_KEY;
    static DFR_Key_type  lastKey    = NO_KEY;
    static uint8_t currSensor       = 0;
    static uint8_t repeat_key       = 0;
    static state_type state         = st_show_temp;
    static TEMP_DATA_TYPE TempOffsetSensor;
    
    ////////////////////////////////////////
    //Poll keys
    tempKey  = keypad.getKey();
    if (tempKey == SAMPLE_WAIT)
    {
        //De-bounce wait
        SET_TIMER1(TIMER_20MS);
        return;
    }

    if (tempKey != NO_KEY)
    {
        if (DEBUG_KEYS)
        {
            sprintf(print_buf,"Key press: %d - Analog value: %d\n", tempKey, analogRead(gc_keypad_pins));
            Serial.print(print_buf);
        }

        if (tempKey != lastKey)
        {
            //Store newly pressed key
            lastKey  = tempKey;
            SET_TIMER1(TIMER_20MS); //Wait 20ms before resampling
            repeat_key=0;
        }
        else
        {
            //If the same key is pressed, don't change state too quickly
            if (repeat_key == 0)
            {
                //First repeat
                SET_TIMER1(TIMER_500MS);
                repeat_key=1;
                return;
            }
            else
            {
                if (repeat_key < 6)
                {
                    //Go normal repeat speed
                    repeat_key=repeat_key+1;
                    SET_TIMER1(TIMER_500MS);
                }
                else
                {
                    //Go faster!
                    SET_TIMER1(TIMER_250MS);
                }
            }
        }
    }
    else
    {
        //No key
        repeat_key = 0;
        lastKey    = tempKey;
    }
    
    ////////////////////////////////////////
    //Menu state machine
    switch (state)
    {
        case st_show_temp:
            switch (tempKey)
            {
                case SELECT_KEY:
                    state = st_change_CoolOn;
                break;
                
                case UP_KEY:
                    currSensor = SensorNext(currSensor);
                    lcd.clear(); //Wipe the screen
                    PrintTempLCD(g_TempReading[currSensor],false,&lcd);
                break;
                
                case DOWN_KEY:
                    currSensor = SensorPrev(currSensor);
                    lcd.clear(); //Wipe the screen
                    PrintTempLCD(g_TempReading[currSensor],false,&lcd);
                break;
                
                default:
                    // state = st_change_CoolOn;
                break;
            }
            g_showtempLCD=currSensor;
            //Print first row only
            snprintf(print_buf, LCD_WIDTH+1, "Temp sensor %d         ",currSensor);
            lcd.setCursor(0,0);
            lcd.print(print_buf);
        break;
        
        case st_change_CoolOn:
            g_showtempLCD=-1; //Dont show temperature for any sensor
            switch (tempKey)
            {
                case SELECT_KEY:
                    state = st_change_CoolOff;
                    //Write value in EEPROM
                    EEPROM.put(currSensor*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+0, g_CoolOnThresh[currSensor]);
                break;
                case RIGHT_KEY:
                    //Increment
                    g_CoolOnThresh[currSensor]=UpdateCoolOn(g_CoolOnThresh[currSensor],g_CoolOffThresh[currSensor],true); //0.5deg Steps
                break;
                case LEFT_KEY:
                    //Decrement
                    g_CoolOnThresh[currSensor]=UpdateCoolOn(g_CoolOnThresh[currSensor],g_CoolOffThresh[currSensor],false); //0.5deg Steps
                break;
                default:
                    //Unsupported key
                    // state = st_show_temp;
                break;
            }
            //First row
            snprintf(print_buf, LCD_WIDTH+1, "CoolOn sensor %d",currSensor);
            lcd.setCursor(0,0);   //First row
            lcd.print(print_buf);
            //Second row
            PrintTempLCD(g_CoolOnThresh[currSensor],false,&lcd);
        break;
    
        case st_change_CoolOff:
            g_showtempLCD=-1; //Dont show temperature for any sensor
            switch (tempKey)
            {
                case SELECT_KEY:
                    //Load offset calibration value
                    TempOffsetSensor = g_OffsetSensor[currSensor];
                    state = st_calib_sensor;
                    //Write value in EEPROM
                    EEPROM.put(currSensor*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+sizeof(TEMP_DATA_TYPE), g_CoolOffThresh[currSensor]);
                break;
                case RIGHT_KEY:
                    //Increment
                    g_CoolOffThresh[currSensor]=UpdateCoolOff(g_CoolOffThresh[currSensor],g_CoolOnThresh[currSensor],true); //0.5deg Steps
                break;
                case LEFT_KEY:
                    //Decrement
                    g_CoolOffThresh[currSensor]=UpdateCoolOff(g_CoolOffThresh[currSensor],g_CoolOnThresh[currSensor],false); //0.5deg Steps
                break;
                default:
                    //Unsupported key
                    // state = st_show_temp;
                break;
            }
            snprintf(print_buf, LCD_WIDTH+1, "CoolOff sensor %d",currSensor);
            lcd.setCursor(0,0);   //First row
            lcd.print(print_buf);
            //Second row
            PrintTempLCD(g_CoolOffThresh[currSensor],false,&lcd);
        break;
        
        case st_calib_sensor: //For offset calibration
            g_showtempLCD=-1; //Dont show temperature for any sensor
            switch (tempKey)
            {
                case SELECT_KEY:
                    //Go back to Main screen
                    lcd.clear(); //Wipe the screen
                    state = st_show_temp;
                    //Write value in DS1820 scratchpad
                    g_ds1820[currSensor]->WriteUserBytes(DS1820_CONFIG_REG,TempOffsetSensor);
                break;
                case RIGHT_KEY:
                    //Increment
                    TempOffsetSensor+=OFFSET_STEP; //Steps
                break;
                case LEFT_KEY:
                    //Decrement
                    TempOffsetSensor-=OFFSET_STEP; //Steps
                break;
                default:
                    //Unsupported key
                    // state = st_show_temp;
                break;
            }
            //Check limits
            if (TempOffsetSensor > MAX_OFF_TEMP)
            {
                TempOffsetSensor = MAX_OFF_TEMP;
            }
            else if (TempOffsetSensor < - MIN_OFF_TEMP)
            {
                TempOffsetSensor = -MIN_OFF_TEMP;
            }
            //Print on LCD
            snprintf(print_buf, LCD_WIDTH+1, "Off calib sensor %d",currSensor);
            lcd.setCursor(0,0);   //First row
            lcd.print(print_buf);
            //Second row
            PrintTempLCD(TempOffsetSensor,false,&lcd);
        break;
        
        default:
            state = st_show_temp;
        break;
    }
    
    return;
}

//Read all temperature sensors and update global temperature variables
inline void read_temp_sensors()
{
    uint8_t sensor;
    uint8_t reading_ok;
    char    print_buf[MAX_BUF_CHARS];
    static uint32_t end_time=0;
    uint32_t start;
        
    //Print debug stuff
    if (DEBUG_PERF)
    {
        start = millis();
        sprintf(print_buf,"Time between calls: %ld\n", start - end_time);
        Serial.print(print_buf);
    }
    
    //////////////////////////////////////////////////////////////////
    //Loop all sensors
    //////////////////////////////////////////////////////////////////
    
    for (sensor=0;sensor<NUM_DS1820_SENSORS;sensor++)
    {
        //////////////////////////////////////////////////////////////////
        //Read temperature
        //////////////////////////////////////////////////////////////////
        //Update temperature variables
        reading_ok = g_ds1820[sensor]->UpdateTemp(USE_CRC);
        //Start temperature conversion again
        g_ds1820[sensor]->StartTemp();
        
        if (reading_ok == true) //Check if sensor present or CRC error
        {
            g_TempReading[sensor]  = g_ds1820[sensor]->GetTemp();
            g_OffsetSensor[sensor] = g_ds1820[sensor]->GetOffset();
            
            if ( (g_OffsetSensor[sensor] > (TEMP_DATA_TYPE) MAX_OFF_TEMP) || (g_OffsetSensor[sensor] < -(TEMP_DATA_TYPE) MIN_OFF_TEMP) )
            {
                //Invalid value reset to 0
                g_OffsetSensor[sensor] = 0;
            }
            else
            {
                //Apply offset (if calibrated)
                g_TempReading[sensor] += g_OffsetSensor[sensor];
            }
            
            //////////////////////////////////////////////////////////////////
            //Control Cooling/Heating
            //////////////////////////////////////////////////////////////////
            /////////
            //Cooling
            if (g_TempReading[sensor] >= g_CoolOnThresh[sensor])
            {
                //If temperature rises above CoolOnThreshold
                //Turn on cooling
                SwitchRelay(g_CoolSwitch[sensor], true);
            }
            else if (g_TempReading[sensor] <= g_CoolOffThresh[sensor])
            {
                //If temperature falls below CoolOffThreshold
                //Turn off cooling
                SwitchRelay(g_CoolSwitch[sensor], false);
            }
            /////////
            //Heating
            if (g_TempReading[sensor] <= g_HeatOnThresh[sensor])
            {
                //If temperature falls below HeatOnThreshold
                //Turn on heating
                SwitchRelay(g_HeatSwitch[sensor], true);
            }
            else if (g_TempReading[sensor] >= g_HeatOffThresh[sensor])
            {
                //If temperature rises above HeatOffThreshold
                //Turn off heating
                SwitchRelay(g_HeatSwitch[sensor], false);
            }
            
            //////////////////////////////////////////////////////////////////
            //Printing
            //////////////////////////////////////////////////////////////////

            bool SignBit;
            
            ///////////////////
            //7-segment display
            int32_t DispTemp32;
            SignBit    = (g_TempReading[sensor] < 0) ? true : false;                     //test most sig bit
            if (TEMP_FAHRENHEIT==0)
            {
                //Format to display in 4 7-segment chars (multiply by 100, then remove fractional bits)
                //Celsius
                DispTemp32 = ( ( (int32_t) (g_TempReading[sensor]) ) * 100 ) >> 4;           //Promote to 32-bits for the 7-seg display
                DispTemp32 = SignBit ? -DispTemp32 : DispTemp32;                             //Complement if negative
                g_disp7seg[sensor]->showNumberDecEx((unsigned)DispTemp32, 0xFF, true, 4, 0);
            }
            else
            {
                //Format to display in 4 7-segment chars (multiply by 10, then remove fractional bits)
                //Fahrenheit
                DispTemp32 = ( ( (int32_t) (celsius2fahrenheit(g_TempReading[sensor])) ) * 10 ) >> 4;                 //Promote to 32-bits for the 7-seg display
                DispTemp32 = SignBit ? -DispTemp32 : DispTemp32;                             //Complement if negative
                g_disp7seg[sensor]->showNumberDecEx((unsigned)DispTemp32, 0x00, true, 4, 0);
            }
        }
        
        //////////////
        //LCD printing
        if (g_showtempLCD == sensor)
        {
            PrintTempLCD(g_TempReading[sensor],reading_ok==false,&lcd);
        }
    }
    
    //Print debug stuff
    if (DEBUG_PERF)
    {
        //Print free ram
        sprintf(print_buf,"Free RAM [bytes]: %d\n", freeRam());
        Serial.print(print_buf);
        //Measure time
        end_time = millis();
        sprintf(print_buf,"Read sensors time: %ld\n", end_time - start);
        Serial.print(print_buf);
    }
}