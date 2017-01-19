//Leonardo Capossio for 'fresca' project
//Basic test of peripherals LCD, 7Seg display and Temperature sensor

#include <LiquidCrystal.h>
#include <TM1637Display.h>
#include <OneWire.h>
#include <DFR_Key.h>
//#include <stdint.h> //For exact integer data types

#define DO_DEBUG 1
#define MAX_BUF_CHARS  64
#define USE_CRC  0

//Relays
#define RELAY_PIN 9

//LCD 16x2 based on the Hitachi HD44780 controller
//initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

//Analog Keypad on the LCD
DFR_Key keypad(0); //Keypad on analog pin A0

//
// 7-segment display
#define CLK 8
#define DIO 7
TM1637Display display(CLK, DIO);

/* DS18S20 Temperature chip i/o */

OneWire  ds0(6);
#define NUM_DS1820_SENSORS 1 //One sensor per wire


//Global variables
int16_t  TempReading[NUM_DS1820_SENSORS] = {0};
byte showtempLCD = 0;
int  CoolOnThresh[NUM_DS1820_SENSORS] = {0};
int  CoolOffThresh[NUM_DS1820_SENSORS] = {0};
int  OffsetSensor[NUM_DS1820_SENSORS] = {0};

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
    display.setBrightness(0x0f);
    display.showNumberDec(1337, true);
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
    keypad.setRate(20);
    Serial.print("Done!");Serial.println();
    
    //Initialize Relays
    Serial.print("Initializing relays...");
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    Serial.print("Done!");Serial.println();
    
    showtempLCD=0; //Show temperature for sensor0
    
    // Initialize Timer1 interrupt
    Serial.print("Initializing timer interrupts ...");
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A  = (unsigned) ((16e6L/256.0)*0.8);         // Compare match register (16MHz/256)*Segs = (16e6/256)*0.8Seg, if using 256 prescaler
    TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS12);    // 256 prescaler 
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
    Serial.print("Done!");Serial.println();
    
    Serial.print("***Starting main program...");Serial.println();
    delay(1000);
    interrupts(); sei();// enable all interrupts
}

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
            ds0.skip();         //SkipROM command, we only have one sensor per wire
            ds0.write(0xBE);    // Read Scratchpad
            Serial.print("  Data = ");
            Serial.print(" ");
            for ( i = 0; i < 9; i++) 
            {
                // Read 9 bytes
                data[i] = ds0.read();
                Serial.print("0x");
                Serial.print(data[i], HEX);
                Serial.print(" ");
            }

            //Immediately restart temperature conversion
            ds0.reset();
            // ds0.select(addr[sensor]); //Select a particular sensor
            ds0.skip();         //SkipROM command, we only have one sensor per wire
            ds0.write(0x44,0);  // Command 0x44, start temperature conversion

            //Process data
            crc_err = (OneWire::crc8(data, 9)==0) ? false : true;
            if (!crc_err)
            {
                Serial.print(" CRC OK ");Serial.println();
                
                LowByte  = data[0]; //Temp low byte
                HighByte = data[1]; //Temp high byte
                TempReading[sensor] = (HighByte << 8) + LowByte;
                TempReading[sensor] += OffsetSensor[sensor]; //Remove offset (if calibrated)
                // SignBit  = TempReading[sensor] & 0x8000;  // test most sig bit
                // TempReading[sensor] = SignBit ?  0xffff0000 | TempReading[sensor] : TempReading[sensor]; //Sign extend if negative to 32-bits
            }
            else
            {
                //There was a CRC error
                Serial.print(" CRC ERROR ");Serial.println();
            }
        }
        else
        {
            Serial.print("***NO SENSOR***");Serial.println();
        }
        
        //////////////////////////////////////////////////////////////////
        //Printing
        //////////////////////////////////////////////////////////////////

        //7-segment
        display.showNumberDecEx((TempReading[sensor]*(100<<4))>>4, 0x10, true); //Format to display in 4 7-segment chars (multiply by 100, then remove fractional bits)
        
        // separate off the whole and fractional portions
        // Whole = TempReading[sensor]>>4;  //Divide by 16 to get the whole part
        // Fract = TempReading[sensor]&0xF; //Leave only the fractional bits
        float TempFloat;

        TempFloat = ((float)TempReading[sensor])*(1.0/16.0); //Can't do a shift, that is bullshit, floting point divide is not cheap man, not cool
        
        //Serial
        sprintf(print_buf," Temperature sensor %d: %+2.2f\xA7 C\n",sensor, TempFloat); //0xA7 is ? in ASCII
        Serial.print(print_buf);

        //LCD
        if (showtempLCD == sensor)
        {
            //First row
            snprintf(print_buf, LCD_WIDTH+1+1, "Temp sensor %d         ",sensor);
            lcd.setCursor(0,0);
            lcd.print(print_buf);
            //Second row
            snprintf(print_buf, LCD_WIDTH+1+1, "%+2.2f\xDF C%s", TempFloat, (crc_err)||(!present) ? "  ERR!     " : "          "); //0xDF is ? in the LCD char set
            lcd.setCursor(0,1);
            lcd.print(print_buf);
        }
    }
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
    int tempKey    = 0;
    static int localKey   = 0;
    static int currSensor = 0;
    static state_type state = st_show_temp;
    char print_buf[MAX_BUF_CHARS];
    
    //Poll keys
    tempKey = keypad.getKey();
    if ((tempKey != SAMPLE_WAIT) && (tempKey != NO_KEY))
    {
        localKey = tempKey;
        
        switch (state)
        {
            case st_show_temp:
                switch (localKey)
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
                        state = st_change_CoolOn;
                    break;
                }
                showtempLCD=currSensor;
                digitalWrite(RELAY_PIN, LOW);
            break;
            
            case st_change_CoolOn:
                showtempLCD=-1; //Dont show temperature for any sensor
                switch (localKey)
                {
                    case SELECT_KEY:
                        state = st_change_CoolOff;
                    break;
                    case RIGHT_KEY:
                        
                    break;
                    case LEFT_KEY:
                        
                    break;
                    default:
                        //Unsupported key
                        state = st_show_temp;
                    break;
                }
                snprintf(print_buf, LCD_WIDTH+1, "CoolOn sensor %d",currSensor);
                lcd.setCursor(0,0);   //First row
                lcd.print(print_buf);
                // snprintf(print_buf, LCD_WIDTH+1, "%c%d.%d\223C",SignBit ? '-' : '+', Whole, Fract);
                lcd.setCursor(0,1);   //Second row
                lcd.print(print_buf);
                digitalWrite(RELAY_PIN, HIGH);
            break;
        
            case st_change_CoolOff:
                showtempLCD=-1; //Dont show temperature for any sensor
                switch (localKey)
                {
                    case SELECT_KEY:
                        state = st_calib_sensor;
                    break;
                    case RIGHT_KEY:
                        
                    break;
                    case LEFT_KEY:
                        
                    break;
                    default:
                        //Unsupported key
                        state = st_show_temp;
                    break;
                }
                snprintf(print_buf, LCD_WIDTH+1, "CoolOff sensor %d",currSensor);
                lcd.setCursor(0,0);   //First row
                lcd.print(print_buf);
                // snprintf(print_buf, LCD_WIDTH+1, "%c%d.%d\223C",SignBit ? '-' : '+', Whole, Fract);
                lcd.setCursor(0,1);   //Second row
                lcd.print(print_buf);
                digitalWrite(RELAY_PIN, LOW);
            break;
            
            case st_calib_sensor: //For offset calibration
                showtempLCD=-1; //Dont show temperature for any sensor
                switch (localKey)
                {
                    case SELECT_KEY:
                        state = st_show_temp;
                    break;
                    case RIGHT_KEY:
                        
                    break;
                    case LEFT_KEY:
                        
                    break;
                    default:
                        //Unsupported key
                        state = st_show_temp;
                    break;
                }
                snprintf(print_buf, LCD_WIDTH+1, "Off calib sensor %d",currSensor);
                lcd.setCursor(0,0);   //First row
                lcd.print(print_buf);
                // snprintf(print_buf, LCD_WIDTH+1, "%c%d.%d\223C",SignBit ? '-' : '+', Whole, Fract);
                lcd.setCursor(0,1);   //Second row
                lcd.print(print_buf);
                digitalWrite(RELAY_PIN, HIGH);
            break;
            
            default:
                state = st_show_temp;
            break;
        }
        
        delay(500); //Don't change state too often
    }
    
}