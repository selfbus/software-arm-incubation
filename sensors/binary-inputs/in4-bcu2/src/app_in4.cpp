/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_in4.h"
#include "com_objs.h"
#include "params.h"
#include <sblib/serial.h>

BCU2 bcu = BCU2();

const byte* params; //!< The parameters of the current channel (4 bytes)
int lastLock[NUM_CHANNELS];

/**
 * Get the type of an input channel.
 * See INPUT_TYPE_xx defines in com_objs.h
 *
 * @param channel - the input channel (0..)
 * @return The type of the input channel.
 */
inline int channelType(int channel)
{
    unsigned char type = (*(bcu.userEeprom))[EE_INPUT1_TYPE + (channel >> 1)];

    if (channel & 1) type >>= 1;
    else type &= 15;

    return type;
}

/**
 * Get the channel value for a switch command.
 *
 * @param objno - the ID of the communication object
 * @param cmd - the command
 */
void switchCommand(int objno, int cmd)
{
    unsigned int value;

    switch (cmd)
    {
    case CMD_CAT_PINCHANGE|CMD_ON:
        value = 1;
        break;

    case CMD_CAT_PINCHANGE|CMD_OFF:
        value = 0;
        break;

    case CMD_CAT_PINCHANGE|CMD_TOGGLE:
        value = !bcu.comObjects->objectRead(objno);
        break;

    default:
        value = 0; // this should never happen
    }

    bcu.comObjects->objectWrite(objno, value);
}

/**
 * The value of an input channel of type "switch" changed.
 *
 * @param channel - the input channel (0..7)
 * @param pinValue - the current value of the input pin (0 or 1)
 */
void switchChannelChanged(int channel, int pinValue)
{
    int cmd;
    int cmdBitOffset;
    // int objVal;

    if (pinValue)  // rising edge
        cmdBitOffset = 2;
    else  // falling edge
        cmdBitOffset = 0;

    // Primary object (1.1, 2.1, ...)
    cmd = (params[2] >> cmdBitOffset) & 3;
    switchCommand(COMOBJ_PRIMARY1 + channel, CMD_CAT_PINCHANGE | cmd);

    // Secondary object (1.2, 2.2, ...)
    cmdBitOffset += 4;
    cmd = (params[2] >> cmdBitOffset) & 3;
    switchCommand(COMOBJ_SECONDARY1 + channel, CMD_CAT_PINCHANGE | cmd);
}

void inputChanged(int channel, int val)
{
    params = channelParams + (channel << 2);

    if (bcu.comObjects->objectRead(COMOBJ_LOCK1 + channel))
    {
        return;
    }

    serial.print("input ");
    serial.print(channel);
    serial.print(" value ");
    serial.println(val);

    if (lastLock[channel])
        return;

    const int type = channelType(channel);
    switch (type)
    {
    case CHANNEL_TYPE_SWITCH:
        switchChannelChanged(channel, val);
        break;

    default:
        break;
    }
}

void channelLockChanged(int channel, bool locked)
{
    ///\todo implement channelLockChanged
}

void objectUpdated(int objno)
{
    if (objno >= COMOBJ_LOCK1 && objno < COMOBJ_LOCK1 + NUM_CHANNELS)
    {
        int channel = objno - COMOBJ_LOCK1;
        int locked = bcu.comObjects->objectRead(objno);

        if (locked != lastLock[channel])
        {
            lastLock[channel] = locked;
            channelLockChanged(channel, locked);
        }
    }
}

void initApplication(void)
{
    int channel;
    for (channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        params = channelParams + (channel << 2);

        const int type = channelType(channel);
        // set the initial state of the lock object
        lastLock[channel] = (params [0] & 0x03) == 0x02;
        bcu.comObjects->objectSetValue(COMOBJ_LOCK1      + channel, 0);

        switch (type) ///\todo implement missing functionality
        {
        case CHANNEL_TYPE_SWITCH:      ; break;
        case CHANNEL_TYPE_DIM:         ; break;
        case CHANNEL_TYPE_JALO:        ; break;
        case CHANNEL_TYPE_DIM_ENCODER: ; break;
        }
    }
}
