/*
 *  dusk.cpp - Handling of the dusk function
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "dusk.h"
#include "weatherstation.h"
#include <sblib/eib.h>

Dusk::Dusk()
: SensorBase()
{
}

void Dusk::Initialize(void)
{
    this->InitializeSensor
        ( userEeprom.getUInt8(0x4577)
        , COM_OBJ_DUSK
        , 0x4578
        , userEeprom.getUInt8(0x4579)
        );
    cycleDayPeriod = timeConversionMinSec(0x459A);
    cyclicDaySend.start(cycleDayPeriod);
    unsigned int dayNightCfg = userEeprom.getUInt8(0x457A);
    if (dayNightCfg)
    {
        unsigned int objValueInvert = 0;
        if (dayNightCfg == 2) objValueInvert = 0x100;

        dayNight.immediateObjNumber = COM_OBJ_DUSK_TOGGLE_DAY_NIGHT | objValueInvert;
        dayNight.cycleTime          = timeConversionMinSec(0x459A);
        dayNight.objNumber          = COM_OBJ_DUSK_SHUTTER_UP_DOWN;
        dayNight.blockObjNo         = COM_OBJ_DUSK_SHUTTER_BLOCK;
        // day limit
        dayNight.upperLimit         = userEeprom.getUInt8(0x457B);
        dayNight.upperLimitTime     = timeConversionMinSec(0x45A8);
        dayNight.sendLimitExceeded  = 1; // send "1"
        // night limit
        dayNight.lowerLimit         = userEeprom.getUInt8(0x457C);
        dayNight.lowerLimitTime     = timeConversionMinSec(0x45A9);
        dayNight.sendLowerDeviation = 2; // send "0"
    }
}

void Dusk::periodic(void)
{
    unsigned int newDusk = _newValue();
    _handleNewValue(newDusk);
    if (dayNight.objNumber >= 0)
    {
        dayNight.periodic(newDusk);
    }
}

unsigned int Dusk::_newValue()
{
    return 0;
}
