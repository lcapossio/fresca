#ifndef FRESCA_GLOBALS_H
#define FRESCA_GLOBALS_H

#include <LiquidCrystal.h>
#include <TM1637Display.h>
#include <DFR_Key.h>
#include <TempController.h>
#include <fresca.h>
#include <fresca_sensor.h>

extern uint8_t         g_showSensLCD;
extern uint8_t         g_showTempHumLCD[NUM_SENSORS];
extern TEMP_DATA_TYPE  g_TempReading[NUM_SENSORS];
extern HUM_DATA_TYPE   g_HumReading[NUM_SENSORS];
extern TEMP_DATA_TYPE  g_CoolOnThresh[NUM_SENSORS];
extern TEMP_DATA_TYPE  g_CoolOffThresh[NUM_SENSORS];
extern TEMP_DATA_TYPE  g_HeatOnThresh[NUM_SENSORS];
extern TEMP_DATA_TYPE  g_HeatOffThresh[NUM_SENSORS];
extern TEMP_DATA_TYPE  g_OffsetSensor[NUM_SENSORS];

extern TM1637Display                  * g_disp7seg[NUM_SENSORS];
extern fresca_sensor                  * g_fresca_sensor;
extern LiquidCrystal                    lcd;
extern DFR_Key                          keypad;
extern TempController<TEMP_DATA_TYPE> * TempControllers[NUM_SENSORS];

#endif
