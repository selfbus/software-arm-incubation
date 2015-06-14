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
#include "switch.h"

/** Command category: input pin changed. */
#define CMD_CAT_PINCHANGE 0x100

/** Command category: lock a channel. */
#define CMD_CAT_LOCK      0x200

/** Command category: unlock a channel. */
#define CMD_CAT_UNLOCK    0x300

/** Command category: bus power on. */
#define CMD_CAT_POWERON   0x400

/** Command: do nothing. */
#define CMD_NONE          0

/** Command: on. */
#define CMD_ON            1

/** Command: off. */
#define CMD_OFF           2

/** Command: toggle. */
#define CMD_TOGGLE        3

/** Command: like rising edge. */
#define CMD_LIKE_RISING   1

/** Command: like falling edge. */
#define CMD_LIKE_FALLING  2

/** Command: current value. */
#define CMD_CURRENT       3

#define EE_SWITCH_BUS_RETURN_NO_ACTION     0
#define EE_SWITCH_BUS_RETURN_CURRENT_STATE 1
#define EE_SWITCH_BUS_RETURN_SEND_ON       2
#define EE_SWITCH_BUS_RETURN_SEND_OFF      3

/*
 * Get the channel value for a switch command.
 *
 * @param objno - the ID of the communication object
 * @param cmd - the command
 */
static void switchCommand(int objno, int channel, int cmd)
{
    unsigned int value;
    unsigned int send = 1;

    switch (cmd)
    {
    case CMD_CAT_PINCHANGE|CMD_ON:
        value = 1;
        break;

    case CMD_CAT_PINCHANGE|CMD_OFF:
        value = 0;
        break;

    case CMD_CAT_PINCHANGE|CMD_TOGGLE:
        value = !objectRead(objno);
        break;
    default:
        send = 0;
    }
    if (send)
    {
        objectWrite(objno, value);
    }
}

void switchChannelChanged(int channel, int pinValue)
{
    int cmd, cmdBitOffset;

    if (pinValue)  // rising edge
        cmdBitOffset = 2;
    else  // falling edge
        cmdBitOffset = 0;

    // Primary object (1.1, 2.1, ...)
    cmd = (params[2] >> cmdBitOffset) & 3;
    switchCommand(COMOBJ_PRIMARY1 + channel, channel, CMD_CAT_PINCHANGE | cmd);

    // Secondary object (1.2, 2.2, ...)
    cmdBitOffset += 4;
    cmd = (params[2] >> cmdBitOffset) & 3;
    switchCommand(COMOBJ_SECONDARY1 + channel, channel + 8, CMD_CAT_PINCHANGE | cmd);
}

void switchPeriodic (int channel)
{
    if (timeout[channel].expired())
    {
        int cyclicCfg = (params [0] & 0x0C) >> 2; // get the cyclic configuration
        int value     = objectRead(channel);
        if ( value && (cyclicCfg & 0x01)) // send cyclic telegram if value is ON
            objectWritten(channel);
        if (!value && (cyclicCfg & 0x02)) // send cyclic telegram if value is OFF
            objectWritten(channel);
        timeout[channel].start(delayTime[channel]);
    }
}

static void _switchSetupDelay (int channel)
{
    int coff = channel & 0x07;
    int timeBase = userEeprom[EE_CHANNEL_TIMING_PARAMS_BASE + ((channel + 1) >> 1)];
    if (! (coff & 0x01)) timeBase >>= 4;
    else                 timeBase  &= 0x0F;

    if (!timeBase || ((channel >= 8) && timeBase == 15))
    {   // for the secondary objects a time base of 15 means no cyclic sending
        delayTime[channel] = 0;
    }
    else
    {
        delayTime[channel] = calculateTime(timeBase, params [1] & 0x7F);
        if ((params [0] & 0x0C) >> 2) // get the cyclic configuration
        {
            timeout[channel].start(delayTime[channel]);
        }
    }
}

void switchSetup(int channel)
{
    unsigned int value;
    int busReturnCfg;

    // setup the delay times for cyclic switching. These calculation need to be done only once
    _switchSetupDelay (channel);
    _switchSetupDelay (channel + 8);

    busReturnCfg = params [0] >> 6;
    // handle bus return function
    switch (busReturnCfg) // bus return config
    {
    case EE_SWITCH_BUS_RETURN_CURRENT_STATE:
        value = inputDebouncer[channel].lastValue();
        switchChannelChanged(channel, value);
        break;
    case EE_SWITCH_BUS_RETURN_SEND_ON:
    case EE_SWITCH_BUS_RETURN_SEND_OFF:
        value = busReturnCfg == EE_SWITCH_BUS_RETURN_SEND_ON ? 1 : 0;
        objectWrite(COMOBJ_PRIMARY1   + channel, value);
        objectWrite(COMOBJ_SECONDARY1 + channel, value);
        break;
    default:
    case EE_SWITCH_BUS_RETURN_NO_ACTION:
        objectSetValue(COMOBJ_PRIMARY1   + channel, 0);
        objectSetValue(COMOBJ_SECONDARY1 + channel, 0);
        break;
    }
}

void switchLock(int state, int channel)
{
    int value;
    int lockAction;
    if (state)
    {
        lockAction = (params [0] & 0x30) >> 4;
        switch(lockAction)
        {
        case EE_INPUT_SWITCH_LOCK_SET_ON:
        case EE_INPUT_SWITCH_LOCK_SET_OFF:
            value = lockAction == EE_INPUT_SWITCH_LOCK_SET_ON ? 1 : 0;
            objectWrite(COMOBJ_PRIMARY1   + channel, value);
            objectWrite(COMOBJ_SECONDARY1 + channel, value);
            break;
        case EE_INPUT_SWITCH_LOCK_TOGGLE:
            value = !objectRead(COMOBJ_PRIMARY1   + channel);
            objectWrite(COMOBJ_PRIMARY1   + channel, value);
            value = !objectRead(COMOBJ_SECONDARY1 + channel);
            objectWrite(COMOBJ_SECONDARY1 + channel, value);
            break;
        case EE_INPUT_SWITCH_LOCK_NO_REACTION:
        default:
            break;
        }
    }
    else
    {
        lockAction = params [3] & 0x03;
        switch(lockAction)
        {
        case EE_INPUT_SWITCH_LOCK_SET_ON:
        case EE_INPUT_SWITCH_LOCK_SET_OFF:
            value = lockAction == EE_INPUT_SWITCH_LOCK_SET_ON ? 1 : 0;
            objectWrite(COMOBJ_PRIMARY1   + channel, value);
            objectWrite(COMOBJ_SECONDARY1 + channel, value);
            break;
        case EE_INPUT_SWITCH_LOCK_SET_CURRENT:
            value = inputDebouncer[channel].lastValue();
            switchChannelChanged(channel, value);
            break;
        case EE_INPUT_SWITCH_LOCK_NO_REACTION:
        default:
            break;
        }
    }
}
