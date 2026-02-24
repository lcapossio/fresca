#include <Arduino.h>
#include "fresca_ui.h"
#include "fresca_globals.h"
#include <fresca_utils.h>
#include <fresca_pinout.h>
#include <EEPROM.h>

void main_menu() {
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
}
