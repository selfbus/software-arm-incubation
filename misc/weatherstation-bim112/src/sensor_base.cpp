/*
 *  sensor_base.cpp - Base class for sensor measurements
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "sensor_base.h"
#include <sblib/eib.h>

SensorBase::SensorBase()
: sendMode (0)
, objNo(-1)
, reqChange(0)
, cyclePeriod(0)
{
}

void SensorBase::InitializeSensor(unsigned char sendMode, char objNo, unsigned int cyclePeriod, unsigned int reqChange)
{
    this->sendMode    = sendMode;
    this->objNo       = objNo;
    this->cyclePeriod = timeConversionMinSec(cyclePeriod);
    this->reqChange   = reqChange;
}

void SensorBase::_handleNewValue(unsigned int newValue)
{
    bool requestSend = true;

    if (sendMode & 0x001)
    {   // send on demand -> only if requested from outside
        requestSend = false;
    }
    if (sendMode & 0x02)
    {   // send on change
        int delta = newValue - lastSentValue;
        if (delta < 0)
            delta += -1;
        if ((delta * 100 / lastSentValue) < reqChange)
            // only send if the changes is more than configured
            requestSend = false;
    }
    if (sendMode & 0x04)
    {   // send cyclic
        if (cyclicSend.stopped() || !cyclicSend.expired())
            // only send if the changes is more than configured
            requestSend = false;
    }
    if (requestSend)
    {
        lastSentValue = newValue;
        objectWrite(objNo, newValue);
        if (sendMode & 0x04)
            cyclicSend.start(cyclePeriod);
    }
    else
        objectSetValue (objNo, newValue);
}

unsigned int SensorBase::timeConversionMinSec(unsigned int address)
{
    unsigned int result = userEeprom.getUInt8(address);
    if (result < 80)
        // the time is in minutes
        result *= 1000*60;
    else
        // the time is kind of seconds (10s -> 80, 20s -> 160, ...)
        result *= 125;
    return result;
}

