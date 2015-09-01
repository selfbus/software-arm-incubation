/*
 *  temperature.cpp - Handling of the temperature sensor
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "temperature.h"
#include "weatherstation.h"

Temperature::Temperature()
: SensorBase()
{
}

void Temperature::Initialize(void)
{
    alignmentOffset = userEeprom.getUInt8(0x45A4);
    InitializeSensor
    ( userEeprom.getUInt8(0x4589)
    , COM_OBJ_TEMPERATURE
    , 0x458A
    , userEeprom.getUInt8(0x458B)
    );
    threshold[0].upperLimit     = userEeprom.getUInt8(0x458D);
    threshold[0].upperLimitTime = 0;
    threshold[0].lowerLimit     = userEeprom.getUInt8(0x459E);
    threshold[0].lowerLimitTime = 0;
    threshold[0].sendLimitExceeded  = userEeprom.getUInt8(0x458E);
    threshold[0].sendLowerDeviation = userEeprom.getUInt8(0x458F);
    threshold[0].cycleTime = timeConversionMinSec(0x459B);

    threshold[1].upperLimit     = userEeprom.getUInt8(0x4591);
    threshold[1].upperLimitTime = 0;
    threshold[1].lowerLimit     = userEeprom.getUInt8(0x459F);
    threshold[1].lowerLimitTime = 0;
    threshold[1].sendLimitExceeded  = userEeprom.getUInt8(0x4592);
    threshold[1].sendLowerDeviation = userEeprom.getUInt8(0x4593);
    threshold[1].cycleTime = timeConversionMinSec(0x459C);

    if (userEeprom.getUInt8(0x458C))
        // check if threshold 1 is active
        threshold[0].objNumber = COM_OBJ_TEMPERATURE_THRESHOLD_1;

    if (userEeprom.getUInt8(0x45AB))
        // check if threshold 2 is active
        threshold[1].objNumber = COM_OBJ_TEMPERATURE_THRESHOLD_2;
}

void Temperature::periodic()
{
    temp = _newValue();
    _handleNewValue(temp);
    threshold[0].periodic(temp);
    threshold[1].periodic(temp);
}

unsigned int Temperature::_newValue()
{
    return 0 + alignmentOffset;
}
