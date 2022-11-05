/*
 * rc_protocol.cpp
 *
 *  Created on: 11.09.2020
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#define _USE_MATH_DEFINES
#include <cmath>
#include "rc_protocol.h"

int bcd2int(byte bcd)
{
    return (((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F));
}

bool bit_is_set(byte x, byte n)
{
    return ((x >> n) & 0x01);
}

RCMessage::RCMessage()
{
    _IsValid = false;
}

RCMessage::~RCMessage()
{

}

RCMessage* RCMessage::GetRCMessageFromTelegram(byte * msg, int msg_len)
{
    RCMessage* ret;
    if ((msg_len >= RCParameterMessage::msgLength) && (msg[0] == RCParameterMessage::msgIdentifier))
    {
        ret = new RCParameterMessage();
        if (ret->Decode(msg, msg_len))
        {
            return ret;
        }

    }
    else if ((msg_len >= RCDisplayMessage::msgLength) && (msg[0] == RCDisplayMessage::msgIdentifier))
    {
        ret = new RCDisplayMessage();
        if (ret->Decode(msg, msg_len))
        {
            return ret;
        }
    }
    return NULL;
}

RCParameterMessage::RCParameterMessage()
{
    _WaterExchangePeriod_days = -1;
    _TapWaterSwitchOnHeight_cm = -1;
    _TapWaterSwitchOnHysteresis_cm = -1;
    _WaterExchangeDuration_min = -1;
    _TapWaterSupplyType = none;
    _FillingLevelMax_cm = -1;
    _ReservoirType = cylindrical;
    _ReservoirArea_m2 = -1;
    _OptionalRelaisFunction = no_function;
    _FilterBackFlushingAutomaticTimerInterval_days = -1;
    _FilterBackFlushingAutomaticTimerDuration_seconds = -1;
    _LevelCalibrationFactory = -1;
    _LevelCalibrationUser = -1;
    _LevelMeasured_cm = -1;
    _LevelCalibrated_cm = -1;
    _Level_m3_Calibrated = -1;
}

void RCParameterMessage::operator=(const RCParameterMessage &msg)
{
    _IsValid = msg._IsValid;
    _WaterExchangePeriod_days = msg._WaterExchangePeriod_days;
    _TapWaterSwitchOnHeight_cm = msg._TapWaterSwitchOnHeight_cm;
    _TapWaterSwitchOnHysteresis_cm = msg._TapWaterSwitchOnHysteresis_cm;
    _WaterExchangeDuration_min = msg._WaterExchangeDuration_min;
    _TapWaterSupplyType = msg._TapWaterSupplyType;
    _FillingLevelMax_cm = msg._FillingLevelMax_cm;
    _ReservoirType = msg._ReservoirType;
    _ReservoirArea_m2 = msg._ReservoirArea_m2;
    _OptionalRelaisFunction = msg._OptionalRelaisFunction;
    _FilterBackFlushingAutomaticTimerInterval_days = msg._FilterBackFlushingAutomaticTimerInterval_days;
    _FilterBackFlushingAutomaticTimerDuration_seconds = msg._FilterBackFlushingAutomaticTimerDuration_seconds;
    _LevelCalibrationFactory = msg._LevelCalibrationFactory;
    _LevelCalibrationUser = msg._LevelCalibrationUser;
    _LevelMeasured_cm = msg._LevelMeasured_cm;
    _LevelCalibrated_cm = msg._LevelCalibrated_cm;
    _Level_m3_Calibrated = msg._Level_m3_Calibrated;
}

bool RCParameterMessage::operator==(const RCParameterMessage &msg)
{
    if (_IsValid != msg._IsValid) return false;
    if (_WaterExchangePeriod_days != msg._WaterExchangePeriod_days) return false;
    if (_TapWaterSwitchOnHeight_cm != msg._TapWaterSwitchOnHeight_cm) return false;
    if (_TapWaterSwitchOnHysteresis_cm != msg._TapWaterSwitchOnHysteresis_cm) return false;
    if (_WaterExchangeDuration_min != msg._WaterExchangeDuration_min) return false;
    if (_TapWaterSupplyType != msg._TapWaterSupplyType) return false;
    if (_FillingLevelMax_cm != msg._FillingLevelMax_cm) return false;
    if (_ReservoirType != msg._ReservoirType) return false;
    if (_ReservoirArea_m2 != msg._ReservoirArea_m2) return false;
    if (_OptionalRelaisFunction != msg._OptionalRelaisFunction) return false;
    if (_FilterBackFlushingAutomaticTimerInterval_days != msg._FilterBackFlushingAutomaticTimerInterval_days) return false;
    if (_FilterBackFlushingAutomaticTimerDuration_seconds != msg._FilterBackFlushingAutomaticTimerDuration_seconds) return false;
    if (_LevelCalibrationFactory != msg._LevelCalibrationFactory) return false;
    if (_LevelCalibrationUser != msg._LevelCalibrationUser) return false;
    if (_LevelMeasured_cm != msg._LevelMeasured_cm) return false;
    if (_LevelCalibrated_cm != msg._LevelCalibrated_cm) return false;
    if (_Level_m3_Calibrated != msg._Level_m3_Calibrated) return false;
    return true;
}

bool RCParameterMessage::operator!=(const RCParameterMessage &msg)
{
    return !operator==(msg);
}

bool RCParameterMessage::Decode(byte * msg, int msg_len)
{
    _IsValid = false;
    if ((msg_len >= RCParameterMessage::msgLength) && (msg[0] == RCParameterMessage::msgIdentifier))
    {
        _WaterExchangePeriod_days = bcd2int(msg[1]);
        _TapWaterSwitchOnHeight_cm = bcd2int(msg[2]) * 5;
        _TapWaterSwitchOnHysteresis_cm = bcd2int(msg[3]) * 2;
        _WaterExchangeDuration_min = bcd2int(msg[4]);
        _TapWaterSupplyType = eTapWaterSupplyType(bcd2int(msg[5]));
        _FillingLevelMax_cm = bcd2int(msg[6]) * 5;
        _ReservoirType = eReservoirType(bcd2int(msg[7]));
        _ReservoirArea_m2 = (float)bcd2int(msg[8]) * 0.1f;
        _OptionalRelaisFunction = eOptionalRelaisFunction(bcd2int(msg[9]));
        _FilterBackFlushingAutomaticTimerInterval_days = bcd2int(msg[10]);
        _FilterBackFlushingAutomaticTimerDuration_seconds = bcd2int(msg[11]) * 10;
        _LevelCalibrationFactory = bcd2int(msg[12]);
        _LevelCalibrationUser = bcd2int(msg[13]);
        _LevelMeasured_cm = bcd2int(msg[14]) + bcd2int(msg[15]) * 100;
        _LevelCalibrated_cm = _LevelCalibrationUser + _LevelMeasured_cm - RC_LEVEL_CALIBRATION_FACTOR;

        switch (_ReservoirType)
        {
            case cylindrical:
            {
                _Level_m3_Calibrated = float(_LevelCalibrated_cm) / 100 * _ReservoirArea_m2;
                break;
            }
            case spherical:
            {
                float r = float(_FillingLevelMax_cm) / 2.0f / 100.0f;
                float h = float(_LevelCalibrated_cm) / 100.0f;
                // V = (1/3)pi*h²(3r-h)
                _Level_m3_Calibrated = ((float)M_PI)/3.0f * (h*h) * (3.0f*r-h);
                break;
            }
            default:
            {
                // this should never happen
                _Level_m3_Calibrated = -1;
                break;
            }
        }
        _Level_m3_Calibrated = truncf(_Level_m3_Calibrated * 10.0f) /10.0f;
        _IsValid = true;
    }
    return _IsValid;
}

RCDisplayMessage::RCDisplayMessage()
{
    _DisplayValue = -1;
    _DisplayUnit = invalid;
    _AlarmBuzzerActive = -1;
    _OptionalRelaisBlinking = -1;
    _OptionalRelaisAlwaysOn = -1;
    _WaterExchangeActive = -1;
    _ManualSwitchedToTapWater = -1;
    _AutomaticallySwitchedToTapWater = -1;
    _OptionalLEDBlinking = -1;
    _OptionalLEDAlwaysOn = -1;

    _DisplayInQubicMeters = -1;
    _DisplayInPercent = -1;
    _OffButtonPressed = -1;
    _OnButtonPressed = -1;
    _unused = -1;
    _PumpActive = -1;
    _TapWaterRefillInputActive = -1;
    _OptionalInputActive = -1;
}

void RCDisplayMessage::operator=(const RCDisplayMessage &msg)
{
    _IsValid = msg._IsValid;
    _DisplayValue = msg._DisplayValue;
    _DisplayUnit = msg._DisplayUnit;
    _AlarmBuzzerActive = msg._AlarmBuzzerActive;
    _OptionalRelaisBlinking = msg._OptionalRelaisBlinking;
    _OptionalRelaisAlwaysOn = msg._OptionalRelaisAlwaysOn;
    _WaterExchangeActive = msg._WaterExchangeActive;
    _ManualSwitchedToTapWater = msg._ManualSwitchedToTapWater;
    _AutomaticallySwitchedToTapWater = msg._AutomaticallySwitchedToTapWater;
    _OptionalLEDBlinking = msg._OptionalLEDBlinking;
    _OptionalLEDAlwaysOn = msg._OptionalLEDAlwaysOn;

    _DisplayInQubicMeters = msg._DisplayInQubicMeters;
    _DisplayInPercent = msg._DisplayInPercent;
    _OffButtonPressed = msg._OffButtonPressed;
    _OnButtonPressed = msg._OnButtonPressed;
    _unused = msg._unused;
    _PumpActive = msg._PumpActive;
    _TapWaterRefillInputActive = msg._TapWaterRefillInputActive;
    _OptionalInputActive = msg._OptionalInputActive;
}

bool RCDisplayMessage::operator==(const RCDisplayMessage &msg)
{
    if (_IsValid != msg._IsValid) return false;
    if (_DisplayValue != msg._DisplayValue) return false;
    if (_DisplayUnit != msg._DisplayUnit) return false;
    if (_AlarmBuzzerActive != msg._AlarmBuzzerActive) return false;
    if (_OptionalRelaisBlinking != msg._OptionalRelaisBlinking) return false;
    if (_OptionalRelaisAlwaysOn != msg._OptionalRelaisAlwaysOn) return false;
    if (_WaterExchangeActive != msg._WaterExchangeActive) return false;
    if (_ManualSwitchedToTapWater != msg._ManualSwitchedToTapWater) return false;
    if (_AutomaticallySwitchedToTapWater != msg._AutomaticallySwitchedToTapWater) return false;
    if (_OptionalLEDBlinking != msg._OptionalLEDBlinking) return false;
    if (_OptionalLEDAlwaysOn != msg._OptionalLEDAlwaysOn) return false;

    if (_DisplayInQubicMeters != msg._DisplayInQubicMeters) return false;
    if (_DisplayInPercent != msg._DisplayInPercent) return false;
    if (_OffButtonPressed != msg._OffButtonPressed) return false;
    if (_OnButtonPressed != msg._OnButtonPressed) return false;
    if (_unused != msg._unused) return false;
    if (_PumpActive != msg._PumpActive) return false;
    if (_TapWaterRefillInputActive != msg._TapWaterRefillInputActive) return false;
    if (_OptionalInputActive != msg._OptionalInputActive) return false;
    return true;
}

bool RCDisplayMessage::operator!=(const RCDisplayMessage &msg)
{
    return !operator==(msg);
}

bool RCDisplayMessage::Decode(byte * msg, int msg_len)
{
    _IsValid = false;
    if ((msg_len >= RCDisplayMessage::msgLength) && (msg[0] == RCDisplayMessage::msgIdentifier))
    {
        _DisplayValue = (float)bcd2int(msg[1]) + 100.0f * (float)bcd2int(msg[2]);

        _AlarmBuzzerActive = bit_is_set(msg[3], 7);
        _OptionalRelaisBlinking = bit_is_set(msg[3], 6);
        _OptionalRelaisAlwaysOn = bit_is_set(msg[3], 5);
        _WaterExchangeActive = bit_is_set(msg[3], 4);

        _ManualSwitchedToTapWater = bit_is_set(msg[3], 3);
        _AutomaticallySwitchedToTapWater = bit_is_set(msg[3], 2);
        _OptionalLEDBlinking = bit_is_set(msg[3], 1);
        _OptionalLEDAlwaysOn = bit_is_set(msg[3], 0);

        _DisplayInQubicMeters = bit_is_set(msg[4], 7);
        _DisplayInPercent = bit_is_set(msg[4], 6);
        _OffButtonPressed = bit_is_set(msg[4], 5);
        _OnButtonPressed = bit_is_set(msg[4], 4);

        _unused = bit_is_set(msg[4], 3); // this bit "flickers"
        _PumpActive = bit_is_set(msg[4], 2);
        _TapWaterRefillInputActive = bit_is_set(msg[4], 1);
        _OptionalInputActive = bit_is_set(msg[4], 0);

        if (_DisplayInQubicMeters == true)
        {
            _DisplayUnit = m3;
            _DisplayValue = _DisplayValue / 10;
        }
        else if (_DisplayInPercent == true)
        {
            _DisplayUnit = percent;
        }
        else
        {
            _DisplayUnit = cm;
        }
        _IsValid = true;
    }

    return _IsValid;
}

RCSwitchDisplayMessage::RCSwitchDisplayMessage()
{

}

bool RCSwitchDisplayMessage::Decode(byte * msg, int msg_len)
{
    // return ((msg_len >= RCSwitchDisplayMessage::msgLength) && (msg[0] == RCSwitchDisplayMessage::msgIdentifier));
    return false;
}

RCSwitchToTapWaterRefillMessage::RCSwitchToTapWaterRefillMessage()
{

}

bool RCSwitchToTapWaterRefillMessage::Decode(byte * msg, int msg_len)
{
    // return ((msg_len >= RCSwitchToTapWaterRefillMessage::msgLength) && (msg[0] == RCSwitchToTapWaterRefillMessage::msgIdentifier));
    return false;
}

RCSwitchToReservoirMessage::RCSwitchToReservoirMessage()
{

}

bool RCSwitchToReservoirMessage::Decode(byte * msg, int msg_len)
{
    // return ((msg_len >= RCSwitchToReservoirMessage::msgLength) && (msg[0] == RCSwitchToReservoirMessage::msgIdentifier));
    return false;
}
