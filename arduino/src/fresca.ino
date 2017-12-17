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
#include <TempController.h>
#include <fresca_pinout.h>
#include <fresca_sensor.h>
#include <fresca_utils.h>
#include <fresca.h>

////////////////////////////////////////
//Global variables
static uint8_t         g_showSensLCD = 0;                  //Set to the Sensor number you want to display on the LCD (0: sensor0)
static uint8_t         g_showTempHumLCD[NUM_SENSORS]= {0}; //0: display temperature, 1: display humidity (for respective sensor)
static TEMP_DATA_TYPE  g_TempReading[NUM_SENSORS]   = {0}; //Last successful temperature reading for respective sensor
static HUM_DATA_TYPE   g_HumReading[NUM_SENSORS]    = {0}; //Last successful humidity reading for respective sensor
static TEMP_DATA_TYPE  g_CoolOnThresh[NUM_SENSORS]  = {0};
static TEMP_DATA_TYPE  g_CoolOffThresh[NUM_SENSORS] = {0};
static TEMP_DATA_TYPE  g_HeatOnThresh[NUM_SENSORS]  = {0};
static TEMP_DATA_TYPE  g_HeatOffThresh[NUM_SENSORS] = {0};
static TEMP_DATA_TYPE  g_OffsetSensor[NUM_SENSORS]  = {0};
////////////////////////////////////////

////////////////////////////////////////
//Define objects
static TM1637Display                  * g_disp7seg[NUM_SENSORS];      //7segment displays
static fresca_sensor                  * g_fresca_sensor;              //Fresca sensor class (contains the pointer to all sensors in the system)
static LiquidCrystal                    lcd(gc_lcd_pins[0], gc_lcd_pins[1], gc_lcd_pins[2], gc_lcd_pins[3], gc_lcd_pins[4], gc_lcd_pins[5]); //(rs, enable, d4, d5, d6, d7) //LCD 16x2 based on the Hitachi HD44780 controller
static DFR_Key                          keypad(gc_keypad_pins);       //Analog Keypad on the LCD
static TempController<TEMP_DATA_TYPE> * TempControllers[NUM_SENSORS]; //Temperature controllers
////////////////////////////////////////

////////////////////////////////////////
//Start main code
////////////////////////////////////////

void loop(void)
{
    lcd.clear();                // Wipe the screen
    interrupts();               // enable all interrupts
    SET_TIMER1(TIMER_20MS);     // Set timer
    TIMSK1 |= (1 << OCIE1A);    // enable timer compare interrupt
    
    Serial.print("***Executing Main Loop...");Serial.println();
    
    uint32_t start_time;
    
    //Update sensors for the first time
    start_time = millis();
    read_temp_sensors();
    
    //MAIN Infinite loop
    while(true)
    {
        if (millis()-start_time >= (TEMP_POLL_MSEC+10))
        {
            //Update sensors
            start_time = millis();
            read_temp_sensors();
        }
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
    Serial.print("Aguantia  ...   ");Serial.println();
    Serial.print("---------------");Serial.println();
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
    //Initialize LCD
    Serial.print("Initializing LCD...");
    lcd.begin(LCD_WIDTH, LCD_HEIGHT,1);
    lcd.setCursor(0,0);
    lcd.print("'fresca' project");
    lcd.setCursor(0,1);
    lcd.print("Aguantia  ...   ");
    Serial.print("Done!");Serial.println();
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
    //Initialize Displays
    Serial.print("Initializing 7-segment displays...");
    for (i = 0; i < NUM_7SEG; i++)
    {
        //Create object
        g_disp7seg[i] = new TM1637Display(gc_7seg_clk_pins,gc_7seg_dio_pins[i]);
        //
        g_disp7seg[i]->setBrightness(0x0f,true); //Turn-on
        g_disp7seg[i]->showNumberDec(1305,true);
    }
    //Blink them
    delay_noInterrupts(400);
    for (i = 0; i < NUM_7SEG; i++)
    {
        g_disp7seg[i]->setBrightness(0x00,false);//Turn-off
        g_disp7seg[i]->showNumberDec(1305,true); //Write the number again to turn off
    }
    delay_noInterrupts(400);
    for (i = 0; i < NUM_7SEG; i++)
    {
        g_disp7seg[i]->setBrightness(0x0f,true); //Turn-on again (end blinking)
        g_disp7seg[i]->showNumberDec(1305,true);
    }
    Serial.print("Done!");Serial.println();
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
    //Initialize temperature sensors
    Serial.print("Initializing temperature sensors...");
    
    g_fresca_sensor = new fresca_sensor(NUM_SENSORS,gc_temp_sens_type,gc_temp_sens_pins,&Serial);
    
    Serial.print("Done!");Serial.println();
    //////////////////////////////////////////////////
    
    //////////////////////////////////////////////////
    //Initialize keypad
    Serial.print("Initializing keypad...");
    keypad.setRate(KEYPAD_REFRESH_RATE); //Sets the sample rate at once every x milliseconds.
    Serial.print("Done!");Serial.println();
    //////////////////////////////////////////////////
    
    g_showSensLCD=0; //Show temperature for sensor0
    
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
        for (i=0; i<NUM_SENSORS; i++)
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
        for (i=0; i<NUM_SENSORS; i++)
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
    //Initialize Temperature controllers (Relay actuators)
    Serial.print("Initializing temperature controllers (relays)...");
    uint8_t pins[2];
    TEMP_DATA_TYPE limits[4];
    TEMP_DATA_TYPE thresholds[4];
    for (i=0;i<NUM_SENSORS;i++)
    {
        thresholds[0]=g_CoolOnThresh[i];thresholds[1]=g_CoolOffThresh[i];
        thresholds[2]=g_HeatOnThresh[i];thresholds[3]=g_HeatOffThresh[i];
        pins[0]=g_CoolerPins[i];pins[1]=g_HeaterPins[i];
        limits[0]=MAX_TEMP;limits[1]=MIN_TEMP;
        limits[2]=MAX_TEMP;limits[3]=MIN_TEMP;
        if (g_CoolerEn[i] && g_HeaterEn[i])
        {
            //Both cooling and heating
            TempControllers[i] = new TempController<TEMP_DATA_TYPE>(TempController_type::Both, pins, thresholds, limits, THRESHOLD_STEP);
        }
        else if (g_CoolerEn[i])
        {
            //Just cooling
            TempControllers[i] = new TempController<TEMP_DATA_TYPE>(TempController_type::Cool, pins, thresholds, limits, THRESHOLD_STEP);
        }
        else if (g_HeaterEn[i])
        {
            //Just heating
            TempControllers[i] = new TempController<TEMP_DATA_TYPE>(TempController_type::Heat, &pins[1], &thresholds[2], &limits[2], THRESHOLD_STEP);
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
                
                
                case RIGHT_KEY:
                case LEFT_KEY:
                    lcd.clear(); //Wipe the screen
                    if (g_showTempHumLCD[currSensor]==0)
                    {
                      if (g_fresca_sensor->GetHumiditySupport(currSensor)) //Check for humidity support
                      {
                        //Switch to displaying humidity for this sensor
                        PrintHumidityLCD(g_HumReading[currSensor],false,&lcd);
                        g_showTempHumLCD[currSensor] = 1;
                      }
                    }
                    else
                    {
                      //Switch to displaying temperature for this sensor
                      PrintTempLCD(g_TempReading[currSensor],false,&lcd);
                      g_showTempHumLCD[currSensor] = 0;
                    }
                break;
                
                default:
                    // state = st_change_CoolOn;
                break;
            }
            g_showSensLCD=currSensor;
            //Print first row only
            snprintf(print_buf, LCD_WIDTH+1, "Temp sensor %d         ",currSensor);
            lcd.setCursor(0,0);
            lcd.print(print_buf);
        break;
        
        case st_change_CoolOn:
            g_showSensLCD=-1; //Dont show temperature for any sensor
            switch (tempKey)
            {
                case SELECT_KEY:
                    state = st_change_CoolOff;
                    //Write value in EEPROM
                    EEPROM.put(currSensor*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+0, g_CoolOnThresh[currSensor]);
                break;
                case RIGHT_KEY:
                    //Increment
                    g_CoolOnThresh[currSensor]=TempControllers[currSensor]->UpdateOnTh(true,0);
                break;
                case LEFT_KEY:
                    //Decrement
                    g_CoolOnThresh[currSensor]=TempControllers[currSensor]->UpdateOnTh(false,0);
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
            g_showSensLCD=-1; //Dont show temperature for any sensor
            switch (tempKey)
            {
                case SELECT_KEY:
                    state = st_change_HeatOn;
                    //Write value in EEPROM
                    EEPROM.put(currSensor*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+sizeof(TEMP_DATA_TYPE), g_CoolOffThresh[currSensor]);
                break;
                case RIGHT_KEY:
                    //Increment
                    g_CoolOffThresh[currSensor]=TempControllers[currSensor]->UpdateOffTh(true,0);
                break;
                case LEFT_KEY:
                    //Decrement
                    g_CoolOffThresh[currSensor]=TempControllers[currSensor]->UpdateOffTh(false,0);
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
        
        case st_change_HeatOn:
            g_showSensLCD=-1; //Dont show temperature for any sensor
            switch (tempKey)
            {
                case SELECT_KEY:
                    state = st_change_HeatOff;
                    //Write value in EEPROM
                    EEPROM.put(currSensor*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+sizeof(TEMP_DATA_TYPE)*2, g_HeatOnThresh[currSensor]);
                break;
                case RIGHT_KEY:
                    //Increment
                    g_HeatOnThresh[currSensor]=TempControllers[currSensor]->UpdateOnTh(true,1);
                break;
                case LEFT_KEY:
                    //Decrement
                    g_HeatOnThresh[currSensor]=TempControllers[currSensor]->UpdateOnTh(false,1);
                break;
                default:
                    //Unsupported key
                    // state = st_show_temp;
                break;
            }
            //First row
            snprintf(print_buf, LCD_WIDTH+1, "HeatOn sensor %d ",currSensor);
            lcd.setCursor(0,0);   //First row
            lcd.print(print_buf);
            //Second row
            PrintTempLCD(g_HeatOnThresh[currSensor],false,&lcd);
        break;
        
        case st_change_HeatOff:
            g_showSensLCD=-1; //Dont show temperature for any sensor
            switch (tempKey)
            {
                case SELECT_KEY:
                    //Load offset calibration value
                    TempOffsetSensor = g_OffsetSensor[currSensor];
                    state = st_calib_sensor;
                    //Write value in EEPROM
                    EEPROM.put(currSensor*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+sizeof(TEMP_DATA_TYPE)*3, g_HeatOffThresh[currSensor]);
                break;
                case RIGHT_KEY:
                    //Increment
                    g_HeatOffThresh[currSensor]=TempControllers[currSensor]->UpdateOffTh(true,1);
                break;
                case LEFT_KEY:
                    //Decrement
                    g_HeatOffThresh[currSensor]=TempControllers[currSensor]->UpdateOffTh(false,1);
                break;
                default:
                    //Unsupported key
                    // state = st_show_temp;
                break;
            }
            snprintf(print_buf, LCD_WIDTH+1, "HeatOff sensor %d",currSensor);
            lcd.setCursor(0,0);   //First row
            lcd.print(print_buf);
            //Second row
            PrintTempLCD(g_HeatOffThresh[currSensor],false,&lcd);
        break;
        
        case st_calib_sensor: //For offset calibration
            g_showSensLCD=-1; //Dont show temperature for any sensor
            switch (tempKey)
            {
                case SELECT_KEY:
                    //Go back to Main screen
                    lcd.clear(); //Wipe the screen
                    state = st_show_temp;
                    //Write value in sensor
                    g_fresca_sensor->SetTempOffset(currSensor,TempOffsetSensor,(uint8_t) DS1820_CONFIG_REG);
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
    
    for (sensor=0;sensor<NUM_SENSORS;sensor++)
    {
        //////////////////////////////////////////////////////////////////
        //Read temperature (and humidity if supported)
        //////////////////////////////////////////////////////////////////
        //Read respective sensor
        TEMP_DATA_TYPE TempReading;
        
        TempReading =  g_fresca_sensor->GetTemp(sensor);
        reading_ok  = (g_fresca_sensor->GetStatus(sensor) == SensorStatus_t::FRESCA_SENS_OK) ? true : false;
        
        if (g_fresca_sensor->GetHumiditySupport(sensor))
        {
          //Humidity is supported
          g_HumReading[sensor] = g_fresca_sensor->GetHumidity(sensor);
        }
        
        if (reading_ok == true) //Check if sensor present or CRC error
        {
            //If reading was good, update internal variables
            g_TempReading[sensor]  = TempReading;
            g_OffsetSensor[sensor] = g_fresca_sensor->GetTempOffset(sensor);
            
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
            //Control Temperature
            //////////////////////////////////////////////////////////////////
            /////////
            
            //Call temperature controller class to update its variables and update actuator's state
            TempControllers[sensor]->UpdateTemp(g_TempReading[sensor]);
            
            //////////////////////////////////////////////////////////////////
            //Printing
            //////////////////////////////////////////////////////////////////

            bool SignBit;
            
            if (sensor < NUM_7SEG) //If 7segment display for this sensor exists
            {
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
        }
        
        //////////////
        //LCD printing
        if (g_showSensLCD == sensor)
        {
            if (g_showTempHumLCD[sensor]==0) //0: display temperature, 1: display humidity
              PrintTempLCD(g_TempReading[sensor],reading_ok==false,&lcd);
            else
              PrintHumidityLCD(g_HumReading[sensor],reading_ok==false,&lcd);
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