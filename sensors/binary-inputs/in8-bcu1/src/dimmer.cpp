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
#include "dimmer.h"

#define DIM_STEP_MASK      0x07
#define DIM_DIRECTION_MASK 0x08

void dimChannelChanged(int channel, int pinValue)
{
    int method          = (params [0] & 0x70) >> 4;
    if (pinValue) // raising edge
    {   // start the time between switch and dim
        timeout[channel].start(delayTime[channel]);
    }
    else // falling edge
    {   // if dimming has already been started -> update the value of the dim object
        if (channelData[channel].dim.started)
        {
            // read dim object value
            unsigned int value  = objectRead(channel + 8)  & ~DIM_STEP_MASK;
            // but only send a STOP telegram if configured to do so
            if (params [0] & 0x08)
                 objectWrite(COMOBJ_SECONDARY1 + channel, value);
            else objectUpdate(COMOBJ_SECONDARY1 + channel, value);
        }
        else
        {
            unsigned int value  = !objectRead(channel);
            if (method == DIMMER_TYPE_TWO_HAND_LIGHTER_ON)
                value = 1;
            if (method == DIMMER_TYPE_TWO_HAND_DARKER_OFF)
                value = 0;
            // send telegram on switching object
            objectWrite(COMOBJ_PRIMARY1 + channel, value);
        }
        timeout[channel    ].stop();
        timeout[channel + 8].stop();
        channelData[channel].dim.started = 0;
    }
}

void dimPeriod(int channel)
{
    if (timeout[channel].expired ())
    {
        unsigned int value;
        int          method = (params [0] & 0x70) >> 4;
        // time between switch and dim expired -> start dimming
        channelData[channel].dim.started = 1;
        value = objectRead(COMOBJ_SECONDARY1 + channel);
        switch (method)
        {
        case DIMMER_TYPE_ONE_HAND:
            value = (~value) & DIM_DIRECTION_MASK;
            break;
        case DIMMER_TYPE_TWO_HAND_LIGHTER_ON:
        case DIMMER_TYPE_TWO_HAND_LIGHTER_TOGGLE:
            value = 0x08;
            break;
        case DIMMER_TYPE_TWO_HAND_DARKER_OFF:
        case DIMMER_TYPE_TWO_HAND_DARKER_TOGGLE:
            value = 0x00;
            break;
        }
        if (value & DIM_DIRECTION_MASK) // dim brighter
            value = (value & DIM_DIRECTION_MASK) | ((params [1] & 0x38) >> 3);
        else                            // dim darker
            value = (value & DIM_DIRECTION_MASK) | ((params [1] & 0x07) >> 0);
        objectWrite(COMOBJ_SECONDARY1 + channel, value);
        // start timeout for dim telegram repitition
        timeout[channel + 8].start (delayTime [channel + 8]);
    }
    if (timeout[channel + 8].expired ())
    {
        // send the current dim telegram again
        objectWritten(channel + 8);
        // restart the timeout
        timeout[channel + 8].start (delayTime [channel + 8]);
    }
}

void dimSetup(int channel)
{
    unsigned int value;

    // Calculate time between switch and dim
    value = userEeprom[EE_CHANNEL_TIMING_PARAMS_BASE + ((channel + 1) >> 1)];
    if (! (channel & 0x01)) value >>= 4;
    else                    value  &= 0x0F;

    delayTime[channel    ] = calculateTime(value, params [2] & 0x7F);
    // Calculate repitition time
    value = userEeprom[EE_CHANNEL_TIMING_PARAMS_BASE + ((channel + 1 + 8) >> 1)];
    if (! (channel & 0x01)) value >>= 4;
    else                    value  &= 0x0F;
    if (! (params [0] & 0x04)) // telegram repitition is disabled
         delayTime[channel + 8] = 0;
    else delayTime[channel + 8] = calculateTime(value, params [3] & 0x7F);
    objectSetValue(COMOBJ_PRIMARY1   + channel, 0);
    objectSetValue(COMOBJ_SECONDARY1 + channel, 0);
    if(params [2] & 0x80) // send a ON  telegram on bus return
        objectWrite(COMOBJ_PRIMARY1 + channel, (unsigned int) 1);
    if(params [3] & 0x80) // send a OFF telegram on bus return
        objectWrite(COMOBJ_PRIMARY1 + channel, (unsigned int) 0);
}

void dimLock(int state, int channel)
{
    int value;
    int lockAction;
    if (state)
    {
        lockAction = (params [1] & 0xC0) >> 6;
        switch(lockAction)
         {
         case EE_INPUT_DIM_LOCK_ON_SET_ON:
         case EE_INPUT_DIM_LOCK_ON_SET_OFF:
             value = lockAction == EE_INPUT_SWITCH_LOCK_SET_ON ? 1 : 0;
             objectWrite(COMOBJ_PRIMARY1   + channel, value);
             break;
         case EE_INPUT_DIM_LOCK_ON_TOGGLE:
             value = !objectRead(COMOBJ_PRIMARY1   + channel);
             objectWrite(COMOBJ_PRIMARY1   + channel, value);
             break;
         case EE_INPUT_DIM_LOCK_NO_REACTION:
         default:
             break;
         }
    }
    else
    {
        if (params [0] & 0x80)
        {
            objectWrite(COMOBJ_PRIMARY1   + channel, (unsigned int) 0);
        }
    }
 }
