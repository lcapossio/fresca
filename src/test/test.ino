/*
*********************************************************************************************
** Author: Leonardo Capossio
** Project: 'fresca'
** Description:
**             Basic test of peripherals LCD, 7Seg, relay, keypad and Temperature sensor
**             Connections in Arduino Mega 2560: 
**             *DS18B20 on pin6
**             *Relay on pin9
**             *Analog Keypad on pin A0
**             *Hitachi HD44780 LCD on pins: 12, 11, 5, 4, 3, 2
**             *TM1637 7-seg on pins: CLK pin8, DIO pin7
**             
**             This program will correctly display temperature on the LCD and 7-seg displays
**             The menu can be navigated by pressing the 'Select' key, which will cycle through:
**             temperature display, CoolOn threshold setting, CoolOff threshold setting and Offset calibration setting
**             The CoolOn and CoolOff settings can be modified by pressing the right/left keys,
**             Which will in turn activate the relay when the temperature falls below CoolOn,
**             and will turn the relay off when the temperature rises above CoolOff
*********************************************************************************************
*/

#include <LiquidCrystal.h>
#include <TM1637Display.h>
#include <OneWire.h>
#include <DFR_Key.h>
#include <EEPROM.h>

#define DO_DEBUG 1
#define MAX_BUF_CHARS  64
#define USE_CRC  0
#define NUM_DS1820_SENSORS 1 //One sensor per wire
#define MAX_TEMP 30*16
#define MIN_TEMP 5*16
#define DEG_0_5 8 //0.5deg*16

//EEPROM
#define EEPROM_MAGIC_VAR_ADDR 0             //Byte variable stored in this location indicates EEPROM has been written previously
#define EEPROM_MAGIC_VAR_VALUE 0x5A
#define EEPROM_START_ADDR 1
#define EEPROM_BLOCKSIZE  sizeof(int16_t)*3 //Each block contains: CoolOn, CoolOff, Offset
#define EEPROM_ADDR_INCR  sizeof(int16_t)

//Relays
#define RELAY_PIN 9

//LCD 16x2 based on the Hitachi HD44780 controller
//initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

//Analog Keypad on the LCD
#define KEYPAD_PIN 0 //A0
DFR_Key keypad(KEYPAD_PIN); //Keypad on analog pin A0

//
// 7-segment display
#define SEVENSEG_CLK 8
#define SEVENSEG_DIO 7
TM1637Display display(SEVENSEG_CLK, SEVENSEG_DIO);

/* DS18S20 Temperature chip i/o */
#define SENSOR0_PIN 6
OneWire ds0(SENSOR0_PIN);
#define TEMP_POLL_SEC 0.8

//Global variables
byte     g_showtempLCD = 0; //Set to the Sensor number you want to display in the LCD (0: sensor0)
int16_t  g_TempReading[NUM_DS1820_SENSORS]   = {0};
int16_t  g_CoolOnThresh[NUM_DS1820_SENSORS]  = {24*16};
int16_t  g_CoolOffThresh[NUM_DS1820_SENSORS] = {25*16};
int16_t  g_OffsetSensor[NUM_DS1820_SENSORS]  = {0};
int      g_CoolSwitch[NUM_DS1820_SENSORS]    = {RELAY_PIN}; //Contains pin number for relay index

//Prints temperature in the second row of the LCD
bool PrintTempLCD(int16_t temp, bool show_error)
{
    // Separate off the whole and fractional portions, since sprintf doesn't support printing floats!!!
    char print_buf[MAX_BUF_CHARS];
    bool SignBit;
    int16_t Whole, Fract;
    
    SignBit  = (temp < 0) ? true : false;      //test most sig bit
    Whole    = SignBit ? -temp : temp;         //Complement if negative
    Fract    = ((Whole&0xF)*100)>>4;           //Leave only the last 2 decimal fractional digits
    Whole    = Whole>>4;                       //Divide by 16 to get the whole part

    //Print in the second row
    snprintf(print_buf, LCD_WIDTH+1, "%c%02u.%02u\xDF C%s", SignBit ? '-':'+', Whole, Fract, show_error ? "  ERR!     " : "          "); //0xDF is *deg* in the LCD char set
    lcd.setCursor(0,1);
    lcd.print(print_buf);
    
    //Always successful
    return true;
}

bool SwitchCooling(byte sensor, bool state)
{
    if (state)
    {
        //Turn on cooling
        digitalWrite(g_CoolSwitch[sensor], HIGH);
    }
    else
    {
        //Turn off cooling
        digitalWrite(g_CoolSwitch[sensor], LOW);
    }
    
    //Always successful
    return true;
}

void setup(void) 
{
    // Disable all interrupts
    noInterrupts(); cli();
    
    //Initialize Serial
    Serial.begin(9600);
    Serial.print("---------------");Serial.println();
    Serial.print("'fresca' test");Serial.println();
    Serial.print("Please wait ...");Serial.println();
    Serial.print("---------------");Serial.println();

    //Initialize LCD
    Serial.print("Initializing LCD...");
    lcd.begin(LCD_WIDTH, LCD_HEIGHT,1);
    lcd.setCursor(0,0);
    lcd.print("'fresca' test");
    lcd.setCursor(0,1);
    lcd.print("Please wait ...");
    Serial.print("Done!");Serial.println();

    //Initialize Display
    Serial.print("Initializing 7-segment displays...");
    display.setBrightness(0x0f,true); //Turn-on
    display.showNumberDec(1305, true);
    delay(400);
    display.setBrightness(0x00,false);//Turn-off
    delay(400);
    display.setBrightness(0x0f,true); //Turn-on
    Serial.print("Done!");Serial.println();

    //Initialize temperature sensors
    Serial.print("Initializing DS18B20 temperature sensors...");
    byte sensor;
    for (sensor=0;sensor<NUM_DS1820_SENSORS;sensor++)
    {
        //Default is 12-bit resolution
        ds0.reset();
        // ds0.select(addr[sensor]); //Select a particular sensor
        ds0.skip();         //SkipROM command, we only have one sensor per wire
        ds0.write(0x44,0);  // Command 0x44, start temperature conversion
    }
    Serial.print("Done!");Serial.println();
    
    /*
    keypad.setRate(x);
    Sets the sample rate at once every x milliseconds.
    */
    Serial.print("Initializing keypad...");
    keypad.setRate(40);
    Serial.print("Done!");Serial.println();
    
    //Initialize Relays
    Serial.print("Initializing relays...");
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    Serial.print("Done!");Serial.println();
    
    g_showtempLCD=0; //Show temperature for sensor0
    
    //Recall EEPROM values
    Serial.print("Recalling EEPROM values ... ");
    byte i;
    if (EEPROM.read(EEPROM_MAGIC_VAR_ADDR) == EEPROM_MAGIC_VAR_VALUE)
    {
        //We have written this EEPROM before, recall the values
        Serial.print("Successfully found magic number, recalling values ...");
        for (i=0; i<NUM_DS1820_SENSORS; i++)
        {
            EEPROM.get(i*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+0, g_CoolOnThresh[i]);
            EEPROM.get(i*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+sizeof(int16_t), g_CoolOffThresh[i]);
            EEPROM.get(i*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+sizeof(int16_t)*2, g_OffsetSensor[i]);
        }
    }
    else
    {
        //We haven't written this EEPROM before, store new default values
        Serial.print("***NOT found magic number, writing default values ...");
        EEPROM.put(EEPROM_MAGIC_VAR_ADDR, (byte) EEPROM_MAGIC_VAR_VALUE); //Write magic number
        
        //Write default values
        for (i=0; i<NUM_DS1820_SENSORS; i++)
        {
            EEPROM.put(i*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+0, g_CoolOnThresh[i]);
            EEPROM.put(i*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+sizeof(int16_t), g_CoolOffThresh[i]);
            EEPROM.put(i*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+sizeof(int16_t)*2, g_OffsetSensor[i]);
        }
    }
    Serial.print("Done!");Serial.println();
    
    // Initialize Timer1 interrupt
    Serial.print("Initializing timer interrupts ...");
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A  = (unsigned) ((16.0e6/256.0)*TEMP_POLL_SEC);         // Compare match register (16MHz/256)*Segs = (16e6/256)*0.8Seg, if using 256 prescaler
    TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS12);    // 256 prescaler 
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
    Serial.print("Done!");Serial.println();
    
    Serial.print("***************************");Serial.println();
    Serial.print("***Starting main program***");Serial.println();
    Serial.print("***************************");Serial.println();
    
    delay(1000);
    interrupts(); sei();// enable all interrupts
}

//////////////////////////////////////////
// timer compare interrupt service routine
ISR(TIMER1_COMPA_vect)
{
    byte i, sensor;
    byte present = 0;
    byte data[9];
    bool crc_err;
    char print_buf[MAX_BUF_CHARS];
    int16_t HighByte, LowByte;
    
    // 750ms is the time needed for temp conversion with 12-bits, timer should execute every 800ms
    
    for (sensor=0;sensor<NUM_DS1820_SENSORS;sensor++)
    {
        //Read temperature
        present = ds0.reset();
        
        if (present)
        {
            //If sensor is present, read and update temperature
            ds0.skip();         //SkipROM command, we only have one sensor per wire
            ds0.write(0xBE);    // Read Scratchpad
            Serial.print("Data =");
            for ( i = 0; i < 9; i++)
            {
                // Read 9 bytes
                data[i] = ds0.read();
                Serial.print(" 0x");
                Serial.print(data[i], HEX);
            }

            //Immediately restart temperature conversion
            ds0.reset();
            // ds0.select(addr[sensor]); //Select a particular sensor
            ds0.skip();         // SkipROM command, we only have one sensor per wire
            ds0.write(0x44,0);  // Command 0x44, start temperature conversion

            //Process data
            crc_err = (OneWire::crc8(data, 9)==0) ? false : true; //CRC equal 0 over the whole scratchpad memory means CRC is correct
            if (!crc_err)
            {
                Serial.print(" - CRC OK");Serial.println();
                
                LowByte  = data[0]; //Temp low byte
                HighByte = data[1]; //Temp high byte
                g_TempReading[sensor] = (HighByte << 8) + LowByte;
                g_TempReading[sensor] += g_OffsetSensor[sensor]; //Remove offset (if calibrated)
            }
            else
            {
                //There was a CRC error, don't update temperature
                Serial.print(" CRC ERROR ");Serial.println();
            }
        }
        else
        {
            //If no sensor present (or error), leave the temperature unchanged
            Serial.print("***NO SENSOR***");Serial.println();
        }

        //////////////////////////////////////////////////////////////////
        //Control Cooling
        //////////////////////////////////////////////////////////////////
        if (g_TempReading[sensor] <= g_CoolOnThresh[sensor])
        {
            //Turn on
            SwitchCooling(sensor, true);
        }
        else if (g_TempReading[sensor] >= g_CoolOffThresh[sensor])
        {
            //Turn off
            SwitchCooling(sensor, false);
        }
        
        //////////////////////////////////////////////////////////////////
        //Printing
        //////////////////////////////////////////////////////////////////
        
        // float TempFloat;
        // TempFloat = ((float)g_TempReading[sensor])*(1.0/16.0); //Can't do a shift, that is bullshit, floting point divide is not cheap man, not cool

        bool SignBit;
        
        //7-segment
        int32_t DispTemp32;
        SignBit    = (g_TempReading[sensor] < 0) ? true : false;              // test most sig bit
        DispTemp32 =( ((int32_t)g_TempReading[sensor]) * 100 ) >> 4;          //Promote to 32-bits for the 7-seg display
        DispTemp32 = SignBit ? -DispTemp32 : DispTemp32;    //Complement if negative
        // sprintf(print_buf,"7-seg debug: %d\n", DispTemp32);
        // Serial.print(print_buf);
        display.showNumberDecEx((unsigned)DispTemp32, 0x10, true, 4, 0); //Format to display in 4 7-segment chars (multiply by 100, then remove fractional bits)
        // display.showNumberDecEx(3333, 0x10, true); //Format to display in 4 7-segment chars (multiply by 100, then remove fractional bits)

        // Separate off the whole and fractional portions, since sprintf doesn't support printing floats!!! SHAME! even if compute intensive it should be supported
        // int16_t Whole; int16_t Fract;
        
        // Whole = SignBit ? -g_TempReading[sensor] : g_TempReading[sensor];  //Complement if negative
        // Fract = ((Whole&0xF)*100)>>4; //Leave only the fractional bits
        // Whole = Whole>>4;  //Divide by 16 to get the whole part
        
        //Serial debug
        // sprintf(print_buf,"Interrupt debug: %u\n", (unsigned) ((16.0e6/256.0)*0.8));
        // Serial.print(print_buf);

        // sprintf(print_buf,"Temp debug: %d %f %f %f\n", g_TempReading[sensor], TempFloat, (float)g_TempReading[sensor], 1.0/16.0);
        // Serial.print(print_buf);
        // sprintf(print_buf,"Temperature sensor %d: %c%02u.%02uC\n", sensor, SignBit ? '-':'+', Whole, Fract); //0xA7 is ? in ASCII
        // Serial.print(print_buf);
        
        //LCD
        if (g_showtempLCD == sensor)
        {
            PrintTempLCD(g_TempReading[sensor],(crc_err)||(!present));
        }
    }
}

int UpdateCoolOn(int currVal, int CoolOffVal, bool inc_dec)
{
    //0.5deg Steps
    if (inc_dec)
    {
        //Increment
        currVal+=DEG_0_5;
    }
    else
    {
        //Decrement
        currVal-=DEG_0_5;
    }
    
    //Check limits
    if (currVal > MAX_TEMP)
    {
        currVal = MAX_TEMP;
    }
    else if (currVal < CoolOffVal+DEG_0_5)
    {
        currVal = CoolOffVal+DEG_0_5;
    }
    
    return currVal;
}

int UpdateCoolOff(int currVal, int CoolOnVal, bool inc_dec)
{
    //0.5deg Steps
    if (inc_dec)
    {
        //Increment
        currVal+=DEG_0_5;
    }
    else
    {
        //Decrement
        currVal-=DEG_0_5;
    }
    
    //Check limits
    if (currVal < MIN_TEMP)
    {
        currVal = MIN_TEMP;
    }
    else if (currVal > CoolOnVal-DEG_0_5)
    {
        currVal = CoolOnVal-DEG_0_5;
    }
    
    return currVal;
}

int SensorNext(int currSensor)
{
    currSensor += currSensor;
    if (currSensor > NUM_DS1820_SENSORS-1)
    {
        currSensor = 0;
    }
    
    return currSensor;
}

int SensorPrev(int currSensor)
{
    currSensor -= currSensor;
    if (currSensor < 0)
    {
        currSensor = NUM_DS1820_SENSORS-1;
    }
    
    return currSensor;
}

enum state_type {st_show_temp,st_change_CoolOn, st_change_CoolOff, st_calib_sensor};

//Keys
//SAMPLE_WAIT -1
//NO_KEY 0
//UP_KEY 3
//DOWN_KEY 4
//LEFT_KEY 2
//RIGHT_KEY 5
//SELECT_KEY 1

void loop(void)
{
    int tempKey             = 0;
    static int lastKey      = 0;
    static int currSensor   = 0;
    static state_type state = st_show_temp;
    char print_buf[MAX_BUF_CHARS];
    
    //Poll keys
    tempKey  = keypad.getKey();
    
    if ((tempKey != SAMPLE_WAIT) && (tempKey != NO_KEY))
    {
        lastKey  = tempKey;
        // sprintf(print_buf,"Key press: %d - Analog value: %d\n", lastKey, analogRead(KEYPAD_PIN));
        Serial.print(print_buf);
    }
    
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
                    currSensor = SensorPrev(currSensor);
                break;
                
                case DOWN_KEY:
                    currSensor = SensorNext(currSensor);
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
            PrintTempLCD(g_CoolOnThresh[currSensor],false);
        break;
    
        case st_change_CoolOff:
            g_showtempLCD=-1; //Dont show temperature for any sensor
            switch (tempKey)
            {
                case SELECT_KEY:
                    state = st_calib_sensor;
                    //Write value in EEPROM
                    EEPROM.put(currSensor*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+sizeof(int16_t), g_CoolOffThresh[currSensor]);
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
            PrintTempLCD(g_CoolOffThresh[currSensor],false);
        break;
        
        case st_calib_sensor: //For offset calibration
            g_showtempLCD=-1; //Dont show temperature for any sensor
            switch (tempKey)
            {
                case SELECT_KEY:
                    state = st_show_temp;
                    //Write value in EEPROM
                    EEPROM.put(currSensor*EEPROM_BLOCKSIZE+EEPROM_START_ADDR+sizeof(int16_t)*2, g_OffsetSensor[currSensor]);
                break;
                case RIGHT_KEY:
                    //Increment
                    g_OffsetSensor[currSensor]+=4; //0.25deg Steps
                break;
                case LEFT_KEY:
                    //Decrement
                    g_OffsetSensor[currSensor]-=4; //0.25deg Steps
                break;
                default:
                    //Unsupported key
                    // state = st_show_temp;
                break;
            }
            snprintf(print_buf, LCD_WIDTH+1, "Off calib sensor %d",currSensor);
            lcd.setCursor(0,0);   //First row
            lcd.print(print_buf);
            //Second row
            PrintTempLCD(g_OffsetSensor[currSensor],false);
        break;
        
        default:
            state = st_show_temp;
        break;
    }
    
    if ((tempKey != SAMPLE_WAIT) && (tempKey != NO_KEY)) delay(500); //Don't change state too often
    
}