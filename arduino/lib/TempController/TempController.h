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
            Header file for TempActuator and TempController classes
            
            
            
            
            
            
            
            
            
            
            
            
            
*********************************************************************************************
*/

#ifndef FRESCA_TEMP_CTRL_H
#define FRESCA_TEMP_CTRL_H

    #include <Arduino.h>
    
    //#define RELAY_ACTIVE_HIGH //uncomment for ACTIVE_HIGH relays
    
    ////////////////////////////////////////
    //Arduino specific MACRO
    #ifndef RELAY_ACTIVE_HIGH
        //Relay is active LOW
        #define SWITCH_ON(X)  digitalWrite(X, LOW);
        #define SWITCH_OFF(X) digitalWrite(X, HIGH);
    #else
        //Relay is active HIGH
        #define SWITCH_ON(X)  digitalWrite(X, HIGH);
        #define SWITCH_OFF(X) digitalWrite(X, LOW);
    #endif
    ////////////////////////////////////////

    enum class TempActuator_type       : uint8_t {Heat=0, Cool=1};
    enum class TempActuator_state_type : uint8_t {Off=0 , On=1};
    enum class TempController_type     : uint8_t {Heat=0, Cool=1, Both=2};
    
    ////////////////////////////////////////
    template <typename Temp_type> //Temp type has to be a fixed point (integer) type with +,-,< and > operators clearly defined
    class TempActuator
    {
        public:
            //
            TempActuator(TempActuator_type ActuatorType,Temp_type OnTh, Temp_type OffTh, uint8_t Actuator_pin,
                         Temp_type UpperThLimit, Temp_type LowerThLimit, Temp_type MinStep);
            //
            void UpdateTemp(Temp_type new_temp);
            void SwitchOn();
            void SwitchOff();
            Temp_type UpdateOnTh(uint8_t inc_dec);
            Temp_type UpdateOffTh(uint8_t inc_dec);
            void CheckOnThLimits();
            void CheckOffThLimits();
            Temp_type GetOnTh();
            Temp_type GetOffTh();
            TempActuator_state_type GetState();
            uint8_t GetOutputEnable();
            void EnableOutput();
            void DisableOutput();
        private:
            TempActuator_state_type _enOutput;      //Write to output or not (internal state will change, but the output will always be OFF)
            TempActuator_state_type _state;         //Internal state
            TempActuator_type _ActuatorType;
            Temp_type _OnTh;
            Temp_type _OffTh;
            Temp_type _LastTemp;
            uint8_t   _Actuator_pin; //Pin for actuator, either On or Off
            Temp_type _UpperThLimit;
            Temp_type _LowerThLimit;
            Temp_type _MinStep; //Temperature threshold update state
            //
            void ControlTemp(Temp_type temperature);
    };
    ////////////////////////////////////////
    ////////////////////////////////////////
    ////////////////////////////////////////
    
    ////////////////////////////////////////
    template <typename Temp_type>
    class TempController
    {
        public:
            TempController(TempController_type ControllerType, uint8_t *Pins, Temp_type * Thresholds, Temp_type * Limits, Temp_type MinStep);
            void UpdateTemp(Temp_type new_temp);
            Temp_type UpdateOnTh (uint8_t inc_dec, uint8_t Actuator);
            Temp_type UpdateOffTh(uint8_t inc_dec, uint8_t Actuator);
            
        private:
            TempController_type _ControllerType;
            TempActuator<Temp_type> * _Actuators[2];
    };
    ////////////////////////////////////////
    
    
    
    
    
    
    
    
    
    
    
    
    
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//TempActuator Methods

template <typename Temp_type>
TempActuator<Temp_type>::TempActuator(TempActuator_type ActuatorType,Temp_type OnTh, Temp_type OffTh, uint8_t Actuator_pin,
                                      Temp_type UpperThLimit, Temp_type LowerThLimit, Temp_type MinStep)
{
    _ActuatorType = ActuatorType;
    _OnTh         = OnTh;
    _OffTh        = OffTh;
    _Actuator_pin = Actuator_pin;
    _UpperThLimit = UpperThLimit;
    _LowerThLimit = LowerThLimit;
    _MinStep      = MinStep;
    _state        = TempActuator_state_type::Off;
    _enOutput     = TempActuator_state_type::On; //By default enable output control
    
    //Arduino specific
    pinMode(_Actuator_pin, OUTPUT);
    SWITCH_OFF(_Actuator_pin); //Switch off output, but don't change the internal state
    //
    
    CheckOnThLimits();
    CheckOffThLimits();
}

template <typename Temp_type>
void TempActuator<Temp_type>::UpdateTemp(Temp_type new_temp)
{
    _LastTemp = new_temp;
    ControlTemp(_LastTemp); //Switch on or off
}

template <typename Temp_type>
uint8_t TempActuator<Temp_type>::GetOutputEnable()
{
    return _enOutput;
}

template <typename Temp_type>
void TempActuator<Temp_type>::EnableOutput()
{
    _enOutput=TempActuator_state_type::On;
}

template <typename Temp_type>
void TempActuator<Temp_type>::DisableOutput()
{
    SWITCH_OFF(_Actuator_pin); //Switch off output, but don't change the internal state
    _enOutput=TempActuator_state_type::Off;
}

template <typename Temp_type>
void TempActuator<Temp_type>::SwitchOn()
{
    if (_enOutput == TempActuator_state_type::On)
    {
        //Only switch on if output is enabled
        SWITCH_ON(_Actuator_pin);
    }
    _state  = TempActuator_state_type::On;
}

template <typename Temp_type>
void TempActuator<Temp_type>::SwitchOff()
{
    SWITCH_OFF(_Actuator_pin);
    _state  = TempActuator_state_type::Off;
}

template <typename Temp_type>
Temp_type TempActuator<Temp_type>::GetOnTh()
{
    return _OnTh;
}

template <typename Temp_type>
Temp_type TempActuator<Temp_type>::GetOffTh()
{
    return _OffTh;
}

template <typename Temp_type>
TempActuator_state_type TempActuator<Temp_type>::GetState()
{
    return _state;
}

//Pass on new temperature, check if actuator needs to be turned on or off
template <typename Temp_type>
void TempActuator<Temp_type>::ControlTemp(Temp_type temperature)
{
    if (_ActuatorType == TempActuator_type::Cool)
    {
        //Cooling
        //Turn ON actuator when temperature rises above the ON threshold,
        //and turn it OFF when the temperature falls below the OFF threshold
        if (temperature >= _OnTh)
        {
            SwitchOn();
        }
        else if (temperature <= _OffTh)
        {
            SwitchOff();
        }
    }
    else
    {
        //Heating
        //Turn ON actuator when temperature falls below the ON threshold,
        //and turn it OFF when the temperature rises above the OFF threshold
        if (temperature <= _OnTh)
        {
            SwitchOn();
        }
        else if (temperature >= _OffTh)
        {
            SwitchOff();
        }
    }
}

template <typename Temp_type>
Temp_type TempActuator<Temp_type>::UpdateOnTh(uint8_t inc_dec)
{
    //Increment/Decrement
    if (inc_dec==true)
    {
        //Increment
        _OnTh+=_MinStep;
    }
    else
    {
        //Decrement
        _OnTh-=_MinStep;
    }
    
    CheckOnThLimits();
    
    return _OnTh;
}

template <typename Temp_type>
Temp_type TempActuator<Temp_type>::UpdateOffTh(uint8_t inc_dec)
{
    //Increment/Decrement
    if (inc_dec==true)
    {
        //Increment
        _OffTh+=_MinStep;
    }
    else
    {
        //Decrement
        _OffTh-=_MinStep;
    }
    
    CheckOffThLimits();
    
    return _OffTh;
}

template <typename Temp_type>
void TempActuator<Temp_type>::CheckOnThLimits()
{
    //Check limits
    if (_ActuatorType == TempActuator_type::Cool)
    {
        //Cooling type
        if (_OnTh > _UpperThLimit)
        {
            _OnTh = _UpperThLimit;
        }
        else if (_OnTh < _OffTh+_MinStep)
        {
            _OnTh = _OffTh+_MinStep;
        }
    }
    else
    {
        //Heating type
        if (_OnTh < _LowerThLimit)
        {
            _OnTh = _LowerThLimit;
        }
        else if (_OnTh > _OffTh-_MinStep)
        {
            _OnTh = _OffTh-_MinStep;
        }
    }
}

template <typename Temp_type>
void TempActuator<Temp_type>::CheckOffThLimits()
{
    //Check limits
    if (_ActuatorType == TempActuator_type::Cool)
    {
        //Cooling type
        if (_OffTh < _LowerThLimit)
        {
            _OffTh = _LowerThLimit;
        }
        else if (_OffTh > _OnTh-_MinStep)
        {
            _OffTh = _OnTh-_MinStep;
        }
    }
    else
    {
        //Heating type
        if (_OffTh > _UpperThLimit)
        {
            _OffTh = _UpperThLimit;
        }
        else if (_OffTh < _OnTh+_MinStep)
        {
            _OffTh = _OnTh+_MinStep;
        }
    }
}



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//TempController Methods

template <typename Temp_type>
TempController<Temp_type>::TempController(TempController_type ControllerType, uint8_t * Pins, Temp_type * Thresholds, Temp_type * Limits, Temp_type MinStep)
{
    _ControllerType = ControllerType;
    
    switch(_ControllerType)
    {
        case TempController_type::Both:
            //Cooling and Heating
            _Actuators[0] = new TempActuator<Temp_type>(TempActuator_type::Cool, Thresholds[0], Thresholds[1], Pins[0], Limits[0], Limits[1], MinStep);
            _Actuators[1] = new TempActuator<Temp_type>(TempActuator_type::Heat, Thresholds[2], Thresholds[3], Pins[1], Limits[2], Limits[3], MinStep);
        break;
        case TempController_type::Cool:
            //Cooling only
            _Actuators[0] = new TempActuator<Temp_type>(TempActuator_type::Cool, Thresholds[0], Thresholds[1], Pins[0], Limits[0], Limits[1], MinStep);
            _Actuators[1] = _Actuators[0]; //In case it is accidentally accessed it won't produce unexpected results
        break;
        case TempController_type::Heat:
            //Heating only
            _Actuators[0] = new TempActuator<Temp_type>(TempActuator_type::Heat, Thresholds[0], Thresholds[1], Pins[0], Limits[0], Limits[1], MinStep);
            _Actuators[1] = _Actuators[0]; //In case it is accidentally accessed it won't produce unexpected results
        break;
        default:
            //ERROR
            _Actuators[0] = new TempActuator<Temp_type>(TempActuator_type::Cool, Thresholds[0], Thresholds[1], Pins[0], Limits[0], Limits[1], MinStep);
            _Actuators[1] = _Actuators[0]; //In case it is accidentally accessed it won't produce unexpected results
        break;
    }
}

template <typename Temp_type>
void TempController<Temp_type>::UpdateTemp(Temp_type new_temp)
{
    switch(_ControllerType)
    {
        case TempController_type::Both:
            //Heating and cooling cannot be On both at once
            //Cooling takes precedence
            
            if (_Actuators[0]->GetState() == TempActuator_state_type::Off)
                _Actuators[0]->DisableOutput(); //Don't activate the output YET
            
            //Check if output needs to be activated
            _Actuators[0]->UpdateTemp(new_temp);
            
            //Check cooling state, and decide what to do
            if (_Actuators[0]->GetState() == TempActuator_state_type::On)
            {
                //Cooling must be activated, so turn off Heating first
                _Actuators[1]->SwitchOff();
                //Now turn Cooling on
                _Actuators[0]->EnableOutput();
                _Actuators[0]->SwitchOn();
            }
            else
            {
                //Cooling is not on, hence we can activate heating if needed
                //See if heating has to be turned on
                _Actuators[1]->UpdateTemp(new_temp);
            }
        break;
        default:
            //Cooling or Heating alone, update temperature
            _Actuators[0]->UpdateTemp(new_temp);
        break;
    }
    
}

template <typename Temp_type>
Temp_type TempController<Temp_type>::UpdateOnTh(uint8_t inc_dec, uint8_t Actuator)
{
    Temp_type newth;
    
    switch(_ControllerType)
    {
        case TempController_type::Both:
            if (Actuator != 0 && Actuator != 1)
            {
                Actuator=0;
            }
            newth = _Actuators[Actuator]->UpdateOnTh(inc_dec);
        break;
        default:
            //Cooling or Heating alone, update temperature
            newth = _Actuators[0]->UpdateOnTh(inc_dec);
        break;
    }
    
    return newth;
}

template <typename Temp_type>
Temp_type TempController<Temp_type>::UpdateOffTh(uint8_t inc_dec, uint8_t Actuator)
{
    Temp_type newth;
    
    switch(_ControllerType)
    {
        case TempController_type::Both:
            if (Actuator != 0 && Actuator != 1)
            {
                Actuator=0;
            }
            newth = _Actuators[Actuator]->UpdateOffTh(inc_dec);
        break;
        default:
            //Cooling or Heating alone, update temperature
            newth = _Actuators[0]->UpdateOffTh(inc_dec);
        break;
    }
    
    return newth;
}
    
    
    
    
    
    
    
    
    
    
    
#endif