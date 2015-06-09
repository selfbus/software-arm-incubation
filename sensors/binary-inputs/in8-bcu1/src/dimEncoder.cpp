/*
 *  Copyright (c) 2014-2015 Stefan Taferner <stefan.taferner@gmx.at>
 *                          Martin Glueck <martin@mangari.org>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib.h>

#include "params.h"
#include "com_objs.h"
#include "dimEncoder.h"

static void dimEncAction(int channel, int action)
{
    switch(action)
    {
    case 1: // same as rising edge
        dimEncChannelChanged(channel, 1); break;
    case 2: // same as falling edge
        dimEncChannelChanged(channel, 0); break;
    case 3: // action based on current switch state
        dimEncChannelChanged(channel, inputDebouncer[channel].lastValue()); break;
    case 0: // no reaction
    default:
        break;
    }
}

void dimEncChannelChanged(int channel, int pinValue)
{
    unsigned int kind = (params [1] & 0xC0) >> 6;
    unsigned int value;
    unsigned int send = 0;
    if (!pinValue && ! (params [0] & 0x08) && (kind != 3))
    {   // stop a running timer on falling edge and store new value into eeprom
        timeout[channel].stop();
        userEeprom [EE_CHANNEL_PARAMS_BASE + (channel << 2) + 2] = objectRead(COMOBJ_PRIMARY1 + channel);
    }
    if (pinValue && ((kind == 0) || (kind == 3)))
    {   // rising edge and action configured
        send  = 1;
        value = params [2];
    }
    else if (!pinValue && ((kind == 1) || (kind == 3)))
    {   // falling edge and action configured
        send = 1;
        if (kind == 1) value = params [2];
        else           value = params [3];
    }
    if (send)
    {
        objectWrite(COMOBJ_PRIMARY1 + channel, value);
    }
    if (pinValue && (kind != 3) && (! (params [0] & 0x08)))
    {   // use as "taster" and changing support configured
        timeout[channel].start (EE_DIMENC_TIME_LONG);
        channelData[channel].dimenc.first = true;
    }
}

void dimEncPeriod(int channel)
{
    if (timeout[channel].expired())
    {   //    switch has been press long enough to start changing
        // or repition time expired
        // change the internal value
        if(channelData[channel].dimenc.first)
        {
            channelData[channel].dimenc.first = false;
            channelData[channel].dimenc.step  = -channelData[channel].dimenc.step;
        }
        channelData[channel].dimenc.value += channelData[channel].dimenc.step;
        // clamp it and send the new value on the bus
        if (channelData[channel].dimenc.value >= 255)
        {
            objectWrite(COMOBJ_PRIMARY1 + channel, (unsigned int) 255);
        }
        else if (channelData[channel].dimenc.value <= 0)
        {
            objectWrite(COMOBJ_PRIMARY1 + channel, (unsigned int) 0);
        }
        else
        {
            objectWrite(COMOBJ_PRIMARY1 + channel, (unsigned int) channelData[channel].dimenc.value);
            // if the value is outside the borders, restart the timeout
            timeout[channel].start(delayTime[channel]);
        }
    }
}

void dimEncoderSetup(int channel)
{
    unsigned int value;
    if (((params [1] & 0xC0) >> 6) != 3)
    {
        value = userEeprom[EE_CHANNEL_TIMING_PARAMS_BASE + ((channel + 1) >> 1)];
        if (! (channel & 0x01)) value >>= 4;
        else                    value  &= 0x0F;
        delayTime[channel] = calculateTime(value, params [3] & 0x7F);
        channelData[channel].dimenc.step = (params [0] & 0xF0) >> 4;
        channelData[channel].dimenc.value = params [2];
    }
    objectSetValue(COMOBJ_PRIMARY1 + channel, 0);
    dimEncAction(channel, (params[1] & 0x0C) >> 2);
}

void dimEncoderLock(int state, int channel)
{
    if (state)
        dimEncAction(channel, params [1] & 0x03);
    else
        dimEncAction(channel, (params [1] & 0x30) >> 4);
}
