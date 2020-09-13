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
    _LevelMeasured = -1;
    _LevelCalibrated = -1;
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
        _LevelMeasured = bcd2int(msg[14]) + bcd2int(msg[15]) * 100;
        _LevelCalibrated = _LevelCalibrationUser + _LevelMeasured - 50 + 1;
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
    _byte4_5 = -1;
    _byte4_4 = -1;
    _ManualSwitchedToTapWater = -1;
    _byte4_2 = -1;
    _Alarm = -1;
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

bool RCDisplayMessage::Decode(byte * msg, int msg_len)
{
    if ((msg_len >= RCDisplayMessage::msgLength) && (msg[0] == RCDisplayMessage::msgIdentifier))
    {
        _DisplayValue = bcd2int(msg[1]) + 100 * bcd2int(msg[2]);

        _byte4_7 = bit_is_set(msg[3], 7);
        _byte4_6 = bit_is_set(msg[3], 6);
        _byte4_5 = bit_is_set(msg[3], 5);
        _byte4_4 = bit_is_set(msg[3], 4);

        _ManualSwitchedToTapWater = bit_is_set(msg[3], 3);
        _byte4_2 = bit_is_set(msg[3], 2);
        _Alarm = bit_is_set(msg[3], 1);
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
