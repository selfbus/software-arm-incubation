/*
 *  wind.cpp - Handling of the wind sensor
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "wind.h"
#include "weatherstation.h"
#include <sblib/analog_pin.h>
#include <sblib/digital_pin.h>

Wind::Wind()
 : SensorBase()
{
    pinMode(WIND_AD_CHANNEL, INPUT_ANALOG);
}

void Wind::Initialize(void)
{
    InitializeSensor
    ( userEeprom.getUInt8(0x457F)
    , COM_OBJ_WIND_VELOCITY
    , 0x4580
    , userEeprom.getUInt8(0x4581)
    );
    threshold[0].upperLimit     = userEeprom.getUInt8(0x4583);
    threshold[0].upperLimitTime = timeConversionMinSec(0x4587);
    threshold[0].lowerLimit     = userEeprom.getUInt8(0x4584);
    threshold[0].lowerLimitTime = timeConversionMinSec(0x4588);
    threshold[0].sendLimitExceeded  = userEeprom.getUInt8(0x4585);
    threshold[0].sendLowerDeviation = userEeprom.getUInt8(0x4586);
    threshold[0].cycleTime = timeConversionMinSec(0x459D);

    threshold[1].upperLimit     = userEeprom.getUInt8(0x45AD);
    threshold[1].upperLimitTime = timeConversionMinSec(0x45B1);
    threshold[1].lowerLimit     = userEeprom.getUInt8(0x45AE);
    threshold[1].lowerLimitTime = timeConversionMinSec(0x45B2);
    threshold[1].sendLimitExceeded  = userEeprom.getUInt8(0x45AF);
    threshold[1].sendLowerDeviation = userEeprom.getUInt8(0x45AC);
    threshold[1].cycleTime = timeConversionMinSec(0x45AC);

    if (userEeprom.getUInt8(0x4582))
        // check if threshold 1 is active
        threshold[0].objNumber = COM_OBJ_WIND_THRESHOLD_1;

    if (userEeprom.getUInt8(0x45AB))
        // check if threshold 2 is active
        threshold[1].objNumber = COM_OBJ_WIND_THRESHOLD_2;
}

void Wind::periodic()
{
    unsigned int newWind = _newValue();
    _handleNewValue(newWind);
    threshold[0].periodic(newWind);
    threshold[1].periodic(newWind);
}

unsigned int Wind::_newValue()
{
    return analogRead(WIND_AD_CHANNEL);
}
