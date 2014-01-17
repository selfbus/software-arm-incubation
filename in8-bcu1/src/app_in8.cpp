/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_in8.h"

#include "com_objs.h"
#include "params.h"

#include <sblib/eib.h>


/*
 * Get the type of an input channel.
 * See INPUT_TYPE_xx defines in com_objs.h
 *
 * @param channel - the input channel (0..)
 * @return The type of the input channel.
 */
inline int channelType(int channel)
{
    unsigned char type = userEeprom[EE_INPUT1_TYPE + (channel >> 1)];

    if (channel & 1) type >>= 1;
    else type &= 15;

    return type;
}

/*
 * The value of an input channel of type "switch" changed.
 *
 * @param channel - the input channel (0..7)
 * @param val - the current value of the pin (0 or 1)
 */
void switchInputChanged(int channel, int val)
{
    objectValues.primary[channel] = val ? 1 : 0;
    objectWritten(COMOBJ_PRIMARY1 + channel);
}

void inputChanged(int channel, int val)
{
    switch (channelType(channel))
    {
    case INPUT_TYPE_SWITCH:
        switchInputChanged(channel, val);
        break;

    default:
        break;
    }
}

void channelLockChanged(int channel, bool locked)
{
    // TODO
}

void objectUpdated(int objno)
{
    if (objno >= COMOBJ_LOCK1 && objno < COMOBJ_LOCK1 + NUM_CHANNELS)
    {
        int channel = objno - COMOBJ_LOCK1;
        int locked = objectValues.lock[channel];

        if (locked != objectValues.lastLock[channel])
        {
            objectValues.lastLock[channel] = locked;
            channelLockChanged(channel, locked);
        }
    }
}
