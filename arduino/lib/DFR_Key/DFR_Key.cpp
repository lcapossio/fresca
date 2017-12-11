#include "Arduino.h"
#include "DFR_Key.h"

//Analog value read has to be the same (within tolerance) in two sampling points set '_refreshRate' milliseconds apart
int DetectKey(int AnalogVal)
{
    //Detect Key
    if      (AnalogVal < RIGHTKEY_ARV)  return RIGHT_KEY;
    else if (AnalogVal < UPKEY_ARV)     return UP_KEY;
    else if (AnalogVal < DOWNKEY_ARV)   return DOWN_KEY;
    else if (AnalogVal < LEFTKEY_ARV)   return LEFT_KEY;
    else if (AnalogVal < SELKEY_ARV)    return SELECT_KEY;
    
    return NO_KEY;
}

DFR_Key::DFR_Key(int analog_pin)
{
    _refreshRate = 20;
    _keyPin      = analog_pin;
    _curInput    = 0;
    _tempKey     = NO_KEY;
    _curKey      = NO_KEY;
    _oldTime     = 0;
    _debouncing  = false;
}

DFR_Key_type DFR_Key::getKey()
{
    if (_debouncing && (millis() > _oldTime + _refreshRate))
    {
        _curInput = analogRead(_keyPin);
        _debouncing = false;
        
        //Detect Key
        _tempKey = DetectKey(_curInput);
        
        //Have to read the same key twice
        if (_curKey == _tempKey )
        {
            //Debouncing succeeded
            return _curKey;
        }
        
        //Key debouncing failed, value changed in debouncing time
        return SAMPLE_WAIT;
        
    }
    else if (!_debouncing)
    {
        _debouncing = true;
         
        _curInput = analogRead(_keyPin);
         
        //Detect Key
        _curKey  = DetectKey(_curInput);
        
        _oldTime = millis();
    }
    return SAMPLE_WAIT;
}

void DFR_Key::setRate(int rate)
{
  _refreshRate = rate;
}