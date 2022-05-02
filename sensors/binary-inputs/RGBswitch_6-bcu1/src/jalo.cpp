/*
 *  Copyright (c) 2014-2015 Stefan Taferner <stefan.taferner@gmx.at>
 *                          Martin Glueck <martin@mangari.org>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "params.h"
#include "com_objs.h"
#include "jalo.h"
#include "app_RGBswitch.h"

void jaloChannelChanged(int channel, int pinValue)
{
    unsigned int ls     = params [3] & 0x08;
    unsigned int value  = bcu.comObjects->objectRead(COMOBJ_SECONDARY1 + channel);
    unsigned int action = (params [3] & 0xF0) >> 4;
    if (!pinValue)
    {   // rising edge
        switch (action)
        {
        case 1 : value = 0;      break; //0 und 1 vertauscht, da Fehler beim parametrieren
        case 2 : value = 1;      break; //festgestellt wurde EXPERIMETELL!!!S
        case 3 : value = !value; break;
        }
        if (! ls)
        {   // sls concept -> send short telegram
            bcu.comObjects->objectWrite(COMOBJ_PRIMARY1   + channel, value);
            // start the time between short and log
            timeout[channel + NUM_CHANNELS].start(delayTime[channel + NUM_CHANNELS]); // time between short and long push
        }
        else
        {   // ls concept -> send long telegram
            bcu.comObjects->objectWrite(COMOBJ_SECONDARY1 + channel, value);
            // start the blade changing time
            timeout[channel].start(delayTime[channel]); //blade change time
        }
    }
    else
    {   // falling edge
        if (timeout[channel].started() && !timeout[channel].expired())
        {
            bcu.comObjects->objectWrite(COMOBJ_PRIMARY1   + channel, value);
        }
        if (timeout[channel + NUM_CHANNELS].started() && !timeout[channel + NUM_CHANNELS].expired())
        {   // falling edge inside short-long time -> no further action
        }
        // in any case, after a falling edge, stop all timers
        timeout[channel    ].stop();
        timeout[channel + NUM_CHANNELS].stop();
    }
}

void jaloPeriod(int channel)
{
    unsigned int ls = params [3] & 0x08;
    unsigned int value;
    if (! ls)
    {   // short-long-short concept
        if (timeout[channel + NUM_CHANNELS].expired())
        {
            value = bcu.comObjects->objectRead(COMOBJ_PRIMARY1 + channel);
            bcu.comObjects->objectWrite(COMOBJ_SECONDARY1 + channel, value);
            // start blade changing time
            timeout[channel].start(delayTime[channel]);
        }
        // check the timeout of the blade changing time to make sure
        // that the timer will be stopped
        // this is a precaution in case the falling edge is not detected within
        // half the range of the system timer
        timeout[channel].expired();
    }
}

void jaloSetup(int channel)
{
    unsigned int value;

   // Calculate blade change time
    value = bcu.userEeprom->getUInt8(EE_CHANNEL_TIMING_PARAMS_BASE + ((channel + 1 + 8) >> 1));
    if (! (channel & 0x01)) value >>= 4;
    else                    value  &= 0x0F;

    delayTime[channel    ] = calculateTime(value, params [2] & 0x7F);
    if (! (params[3] & 0x08)) // Short Long Short
    {
        // Calculate time between short and long push
        value = bcu.userEeprom->getUInt8(EE_CHANNEL_TIMING_PARAMS_BASE + ((channel + 1) >> 1));
        if (! (channel & 0x01)) value >>= 4;
        else                    value  &= 0x0F;

        delayTime[channel + NUM_CHANNELS] = calculateTime(value, params [1] & 0x7F);
    }
    else
        delayTime[channel + NUM_CHANNELS] = 0;
    // handle bus return
    switch ((params [0] & 0xC0) >> 6)
    {
    case EE_JALO_BUS_RETURN_DOWN: bcu.comObjects->objectWrite(COMOBJ_SECONDARY1 + channel, (unsigned int) 0); break;
    case EE_JALO_BUS_RETURN_UP:   bcu.comObjects->objectWrite(COMOBJ_SECONDARY1 + channel, (unsigned int) 1); break;
    case EE_JALO_BUS_RETURN_NO_ACTION:
    default:
        bcu.comObjects->objectSetValue(COMOBJ_SECONDARY1 + channel, 0);
        break;
    }
    bcu.comObjects->objectSetValue(COMOBJ_PRIMARY1 + channel, 0);
}

void jaloLock(int state, int channel)
{
    int value;
    int lockAction;
    if (state)
    {
        lockAction = (params [0] & 0x30) >> 4;
        switch(lockAction)
        {
        case EE_JALO_LOCK_SET_DOWN:
        case EE_JALO_LOCK_SET_UP:
            value = lockAction == EE_JALO_LOCK_SET_UP ? 1 : 0;
            bcu.comObjects->objectWrite(COMOBJ_SECONDARY1 + channel, value);
            break;
        case EE_JALO_LOCK_TOGGLE:
            value = !bcu.comObjects->objectRead(COMOBJ_SECONDARY1 + channel);
            bcu.comObjects->objectWrite(COMOBJ_SECONDARY1 + channel, value);
            break;
        case EE_JALO_LOCK_NO_REACTION:
        default:
            break;
        }
    }
    else
    {
        lockAction = (params [0] & 0x0C) >> 2;
        switch(lockAction)
        {
        case EE_JALO_LOCK_SET_DOWN:
        case EE_JALO_LOCK_SET_UP:
            value = lockAction == EE_JALO_LOCK_SET_UP ? 1 : 0;
            bcu.comObjects->objectWrite(COMOBJ_SECONDARY1 + channel, value);
            break;
        case EE_JALO_LOCK_TOGGLE:
            value = !bcu.comObjects->objectRead(COMOBJ_SECONDARY1 + channel);
            bcu.comObjects->objectWrite(COMOBJ_SECONDARY1 + channel, value);
            break;
        case EE_JALO_LOCK_NO_REACTION:
        default:
            break;
        }

    }
}
