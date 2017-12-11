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
    
#endif