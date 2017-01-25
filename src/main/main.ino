#include <LiquidCrystal.h>
#include <TM1637Display.h>
#include <OneWire.h>

#define USE_CRC 0

// LCD=======================================================
//initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

//
// 7-segment display
#define CLK 8
#define DIO 7
TM1637Display display(CLK, DIO);

/* DS18S20 Temperature chip i/o */

OneWire  ds0(6);
#define NUM_DS1820_SENSORS 1 //One sensor per wire

void setup(void) 
{
  //Initialize Serial
  Serial.begin(9600);
  Serial.print("'fresca' test");Serial.println();
  Serial.print("Please wait ...");Serial.println();
  Serial.print("---------------");Serial.println();
    
  //Initialize LCD
  lcd.begin(LCD_WIDTH, LCD_HEIGHT,1);
  lcd.setCursor(0,0);
  lcd.print("'fresca' test");
  lcd.setCursor(0,1);
  lcd.print("Please wait ...");
  
  //Initialize Display
  display.setBrightness(0x0f);
  display.showNumberDec(1337, true);
  
  //Initialize temperature sensors
  byte sensor;
  for (sensor=0;sensor<NUM_DS1820_SENSORS;sensor++)
  {
      //Default is 12-bit resolution
      ds0.reset();
      // ds0.select(addr[sensor]); //Select a particular sensor
      ds0.skip();         //SkipROM command, we only have one sensor per wire
      ds0.write(0x44,0);  // Command 0x44, start temperature conversion
  }
  
  delay(1000);
}

int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;
char print_buf[LCD_WIDTH];

void loop(void) 
{
  byte i, sensor;
  byte present = 0;
  byte data[9];

  for (sensor=0;sensor<NUM_DS1820_SENSORS;sensor++)
  {
    //Read temperature
    present = ds0.reset();
    ds0.skip(); //SkipROM command, we only have one sensor per wire
    ds0.write(0xBE);         // Read Scratchpad
    
    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" ");
    for ( i = 0; i < 9; i++) 
    {
      // Read 9 bytes
      data[i] = ds0.read();
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    
    //Immediately restart temperature conversion
    ds0.reset();
    // ds0.select(addr[sensor]); //Select a particular sensor
    ds0.skip();         //SkipROM command, we only have one sensor per wire
    ds0.write(0x44,0);  // Command 0x44, start temperature conversion

    //Process data
    Serial.print(" CRC=");
    Serial.print(OneWire::crc8(data, 8), HEX);
    Serial.println();

    LowByte  = data[0]; //Temp low byte
    HighByte = data[1]; //Temp high byte
    TReading = (HighByte << 8) + LowByte;
    SignBit  = TReading & 0x8000;  // test most sig bit
    //Convert to always positive number
    if (SignBit) // negative
    {
      TReading = (TReading ^ 0xffff) + 1; // 2's comp
    }

    // separate off the whole and fractional portions
    Whole = TReading>>4;  //Divide by 16 to get the whole part
    Fract = TReading&0xF; //Leave only the fractional bits
    
    //////////////////////////////////////////////////////////////////
    //Printing
    //////////////////////////////////////////////////////////////////
    
    //7-segment
    display.showNumberDecEx((TReading*100)>>4, 0x10, true); //Format to display in 4 7-segment chars

    //Serial
    sprintf(print_buf," Temperature sensor %d: %c%d.%d�C\n",sensor,SignBit ? '-' : '+', Whole, Fract);
    Serial.print(print_buf);
    
    //LCD
    sprintf(print_buf, "Temperature sensor %d:",sensor);
    lcd.setCursor(0,0);
    lcd.print(print_buf);
    sprintf(print_buf, "%c%d.%d�C",SignBit ? '-' : '+', Whole, Fract);
    lcd.setCursor(0,1);
    lcd.print(print_buf);
  }
  
  delay(1000);     // maybe 750ms is enough, maybe not, for 12-bit conversion
}