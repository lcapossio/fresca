#include <Arduino.h>
#include "fresca_control.h"
#include "fresca_globals.h"
#include <fresca_utils.h>
#include <fresca_pinout.h>
#include <fresca_link.h>

void read_temp_sensors()
{
    uint8_t sensor;
    uint8_t reading_ok;
    char    print_buf[MAX_BUF_CHARS];
    static uint32_t end_time=0;
    uint32_t start;
    uint8_t turn_on_pump = false;
        
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
            
            //Check if cooling actuator was turned on, if it was turn on the pump
            if (TempControllers[sensor]->GetState() == TempController_state_type::Cooling)
            {
              turn_on_pump = true;
            }
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
    
    //////////////
    //Water Pump
    //If any of the Cooling actuators was turned on, turn on the water pump
    if (g_WaterPumpEn[0]!=0)
    {
      if (turn_on_pump)
      {
        //Turn the pump on
        digitalWrite(g_WaterPump[0], RELAY_ON);
      }
      else
      {
        //Turn the pump off
        digitalWrite(g_WaterPump[0], RELAY_OFF);
      }
    }
    //////////////
    
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
