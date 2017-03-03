#ifndef DFR_Key_h
#define DFR_Key_h

#include "Arduino.h"

#define SAMPLE_WAIT -1
#define NO_KEY      0
#define SELECT_KEY  1
#define LEFT_KEY    2
#define UP_KEY      3
#define DOWN_KEY    4
#define RIGHT_KEY   5

//Analog values
#define RIGHTKEY_ARV 50
#define UPKEY_ARV    250
#define DOWNKEY_ARV  450
#define LEFTKEY_ARV  650
#define SELKEY_ARV   850
#define NOKEY_ARV    1023

class DFR_Key
{
  public:
    DFR_Key(int);
    int getKey();
    void setRate(int);
  private:
    int _refreshRate;
    int _keyPin;
    int _keyIn;
    int _curInput;
    int _tempKey;
    int _curKey;
    int _prevKey;
    boolean _debouncing;
    unsigned long _oldTime;
};

#endif