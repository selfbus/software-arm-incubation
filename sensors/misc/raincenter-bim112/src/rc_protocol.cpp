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

}

RCMessage::~RCMessage()
{

}

bool RCMessage::Decode(byte * msg, int msg_len)
{
    return false;
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
    _AutomaticTimerInterval_days = -1;
    _AutomaticTimerDuration_seconds = -1;
    _LevelCalibrationFactory = -1;
    _LevelCalibrationUser = -1;
    _LevelMeasured_cm = -1;
    _LevelCalibrated_cm = -1;
    _Level_m3_Calibrated = -1;
}

void RCParameterMessage::operator=(const RCParameterMessage &msg)
{
    _WaterExchangePeriod_days = msg._WaterExchangePeriod_days;
    _TapWaterSwitchOnHeight_cm = msg._TapWaterSwitchOnHeight_cm;
    _TapWaterSwitchOnHysteresis_cm = msg._TapWaterSwitchOnHysteresis_cm;
    _WaterExchangeDuration_min = msg._WaterExchangeDuration_min;
    _TapWaterSupplyType = msg._TapWaterSupplyType;
    _FillingLevelMax_cm = msg._FillingLevelMax_cm;
    _ReservoirType = msg._ReservoirType;
    _ReservoirArea_m2 = msg._ReservoirArea_m2;
    _OptionalRelaisFunction = msg._OptionalRelaisFunction;
    _AutomaticTimerInterval_days = msg._AutomaticTimerInterval_days;
    _AutomaticTimerDuration_seconds = msg._AutomaticTimerDuration_seconds;
    _LevelCalibrationFactory = msg._LevelCalibrationFactory;
    _LevelCalibrationUser = msg._LevelCalibrationUser;
    _LevelMeasured_cm = msg._LevelMeasured_cm;
    _LevelCalibrated_cm = msg._LevelCalibrated_cm;
    _Level_m3_Calibrated = msg._Level_m3_Calibrated;
}

bool RCParameterMessage::operator==(const RCParameterMessage &msg)
{
    if (_WaterExchangePeriod_days != msg._WaterExchangePeriod_days) return false;
    if (_TapWaterSwitchOnHeight_cm != msg._TapWaterSwitchOnHeight_cm) return false;
    if (_TapWaterSwitchOnHysteresis_cm != msg._TapWaterSwitchOnHysteresis_cm) return false;
    if (_WaterExchangeDuration_min != msg._WaterExchangeDuration_min) return false;
    if (_TapWaterSupplyType != msg._TapWaterSupplyType) return false;
    if (_FillingLevelMax_cm != msg._FillingLevelMax_cm) return false;
    if (_ReservoirType != msg._ReservoirType) return false;
    if (_ReservoirArea_m2 != msg._ReservoirArea_m2) return false;
    if (_OptionalRelaisFunction != msg._OptionalRelaisFunction) return false;
    if (_AutomaticTimerInterval_days != msg._AutomaticTimerInterval_days) return false;
    if (_AutomaticTimerDuration_seconds != msg._AutomaticTimerDuration_seconds) return false;
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
    if ((msg_len >= RCParameterMessage::msgLength) && (msg[0] == RCParameterMessage::msgIdentifier))
    {
        _WaterExchangePeriod_days = bcd2int(msg[1]);
        _TapWaterSwitchOnHeight_cm = bcd2int(msg[2]) * 5;
        _TapWaterSwitchOnHysteresis_cm = bcd2int(msg[3]) * 2;
        _WaterExchangeDuration_min = bcd2int(msg[4]);
        _TapWaterSupplyType = eTapWaterSupplyType(bcd2int(msg[5]));
        _FillingLevelMax_cm = bcd2int(msg[6]) * 5;
        _ReservoirType = eReservoirType(bcd2int(msg[7]));
        _ReservoirArea_m2 = bcd2int(msg[8]) * 0.1;
        _OptionalRelaisFunction = eOptionalRelaisFunction(bcd2int(msg[9]));
        _AutomaticTimerInterval_days = bcd2int(msg[10]);
        _AutomaticTimerDuration_seconds = bcd2int(msg[11]) * 10;
        _LevelCalibrationFactory = bcd2int(msg[12]);
        _LevelCalibrationUser = bcd2int(msg[13]);
        _LevelMeasured_cm = bcd2int(msg[14]) + bcd2int(msg[15]) * 100;
        //_LevelCalibrated_cm = _LevelCalibrationUser + _LevelMeasured_cm - RC_LEVEL_CALIBRATION_FACTOR + 1;
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
                float r = float(_FillingLevelMax_cm) / 2 / 100;
                float h = float(_LevelCalibrated_cm) / 100;
                // V = (1/3)pi*h²(3r-h)
                _Level_m3_Calibrated = M_PI/3 * (h*h) * (3*r-h);
                break;
            }
            default:
            {
                // this should never happen
                _Level_m3_Calibrated = -1;
                break;
            }
        }
        _Level_m3_Calibrated = trunc(_Level_m3_Calibrated * 10) /10;
        return true;
    }
    else
    {
        return false;
    }
}

RCDisplayMessage::RCDisplayMessage()
{
    _DisplayValue = -1;
    _DisplayUnit = invalid;
    _byte4_7 = -1;
    _byte4_6 = -1;
    _Alarm_2 = -1;
    _byte4_4 = -1;
    _ManualSwitchedToTapWater = -1;
    _AutomaticallySwitchedToTapWater = -1;
    _Alarm_1 = -1;
    _byte4_0 = -1;

    _QubicMeters = -1;
    _Percent = -1;
    _byte5_5 = -1;
    _byte5_4 = -1;
    _byte5_3 = -1;
    _byte5_2 = -1;
    _byte5_1 = -1;
    _byte5_0 = -1;
}

void RCDisplayMessage::operator=(const RCDisplayMessage &msg)
{
    _DisplayValue = msg._DisplayValue;
    _DisplayUnit = msg._DisplayUnit;
    _byte4_7 = msg._byte4_7;
    _byte4_6 = msg._byte4_6;
    _Alarm_2 = msg._Alarm_2;
    _byte4_4 = msg._byte4_4;
    _ManualSwitchedToTapWater = msg._ManualSwitchedToTapWater;
    _AutomaticallySwitchedToTapWater = msg._AutomaticallySwitchedToTapWater;
    _Alarm_1 = msg._Alarm_1;
    _byte4_0 = msg._byte4_0;

    _QubicMeters = msg._QubicMeters;
    _Percent = msg._Percent;
    _byte5_5 = msg._byte5_5;
    _byte5_4 = msg._byte5_4;
    _byte5_3 = msg._byte5_3;
    _byte5_2 = msg._byte5_2;
    _byte5_1 = msg._byte5_1;
    _byte5_0 = msg._byte5_0;
}

bool RCDisplayMessage::operator==(const RCDisplayMessage &msg)
{
    if (_DisplayValue != msg._DisplayValue) return false;
    if (_DisplayUnit != msg._DisplayUnit) return false;
    if (_byte4_7 != msg._byte4_7) return false;
    if (_byte4_6 != msg._byte4_6) return false;
    if (_Alarm_2 != msg._Alarm_2) return false;
    if (_byte4_4 != msg._byte4_4) return false;
    if (_ManualSwitchedToTapWater != msg._ManualSwitchedToTapWater) return false;
    if (_AutomaticallySwitchedToTapWater != msg._AutomaticallySwitchedToTapWater) return false;
    if (_Alarm_1 != msg._Alarm_1) return false;
    if (_byte4_0 != msg._byte4_0) return false;

    if (_QubicMeters != msg._QubicMeters) return false;
    if (_Percent != msg._Percent) return false;
    if (_byte5_5 != msg._byte5_5) return false;
    if (_byte5_4 != msg._byte5_4) return false;
    if (_byte5_3 != msg._byte5_3) return false;
    if (_byte5_2 != msg._byte5_2) return false;
    if (_byte5_1 != msg._byte5_1) return false;
    if (_byte5_0 != msg._byte5_0) return false;
    return true;
}

bool RCDisplayMessage::operator!=(const RCDisplayMessage &msg)
{
    return !operator==(msg);
}

bool RCDisplayMessage::Decode(byte * msg, int msg_len)
{
    if ((msg_len >= RCDisplayMessage::msgLength) && (msg[0] == RCDisplayMessage::msgIdentifier))
    {
        _DisplayValue = bcd2int(msg[1]) + 100 * bcd2int(msg[2]);

        _byte4_7 = bit_is_set(msg[3], 7);
        _byte4_6 = bit_is_set(msg[3], 6);
        _Alarm_2 = bit_is_set(msg[3], 5);
        _byte4_4 = bit_is_set(msg[3], 4);

        _ManualSwitchedToTapWater = bit_is_set(msg[3], 3);
        _AutomaticallySwitchedToTapWater = bit_is_set(msg[3], 2);
        _Alarm_1 = bit_is_set(msg[3], 1);
        _byte4_0 = bit_is_set(msg[3], 0);

        _QubicMeters = bit_is_set(msg[4], 7);
        _Percent = bit_is_set(msg[4], 6);
        _byte5_5 = bit_is_set(msg[4], 5);
        _byte5_4 = bit_is_set(msg[4], 4);

        _byte5_3 = bit_is_set(msg[4], 3); // this bit "flickers"
        _byte5_2 = bit_is_set(msg[4], 2);
        _byte5_1 = bit_is_set(msg[4], 1);
        _byte5_0 = bit_is_set(msg[4], 0);

        if (_QubicMeters == true)
        {
            _DisplayUnit = m3;
            _DisplayValue = _DisplayValue / 10;
        }
        else if (_Percent == true)
        {
            _DisplayUnit = percent;
        }
        else
        {
            _DisplayUnit = cm;
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool RCDisplayMessage::IsSwitchedToTapWater()
{
    return ManualSwitchedToTapWater() || AutomaticallySwitchedToTapWater();
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
