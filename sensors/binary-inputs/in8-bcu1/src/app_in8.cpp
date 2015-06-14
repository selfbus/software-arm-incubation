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

#include "switch.h"
#include "dimmer.h"
#include "jalo.h"
#include "dimEncoder.h"

// The parameters of the current channel (4 bytes)
const byte* params;
Timeout timeout[NUM_CHANNELS * 2];
unsigned int delayTime[NUM_CHANNELS * 2];
int lastLock[NUM_CHANNELS];
ChannelData channelData[NUM_CHANNELS];


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

    if (channel & 1) type >>= 4;
    else type &= 15;

    return type;
}

/* handle a change of an input pin */
void inputChanged(int channel, int val)
{
    params = channelParams + (channel << 2);
    if (objectRead(COMOBJ_LOCK1 + channel)) // check if channel is currently locked
        return;

    const int type = channelType(channel);
    switch (type)
    {
    case CHANNEL_TYPE_SWITCH:
        switchChannelChanged(channel, val); break;
    case CHANNEL_TYPE_DIM:
    	dimChannelChanged(channel, val);    break;
    case CHANNEL_TYPE_JALO:
    	jaloChannelChanged(channel, val);    break;
    case CHANNEL_TYPE_DIM_ENCODER:
    	dimEncChannelChanged(channel, val); break;
    case CHANNEL_TYPE_SCENE_NOSTORE:
    case CHANNEL_TYPE_SCENE_STORE:
    case CHANNEL_TYPE_TEMP_ENCODER:
    case CHANNEL_TYPE_LUM_ENCODER:
    case CHANNEL_TYPE_COUNT_IMPULSE_1:
    case CHANNEL_TYPE_COUNT_IMPULSE_2:
    default:
        break;
    }
}

void channelLockChanged(int channel, unsigned int value)
{
    params = channelParams + (channel << 2);
    int lockPolarity = params [0] & 0x03;
    if (lockPolarity)
    {
        const int type = channelType(channel);
    	if (lockPolarity == 2) value = !value; // inverted lock logic
        switch (type)
        {
        case CHANNEL_TYPE_SWITCH:      switchLock(value, channel);     break;
        case CHANNEL_TYPE_DIM:         dimLock(value, channel);        break;
        case CHANNEL_TYPE_JALO:        jaloLock(value, channel);       break;
        case CHANNEL_TYPE_DIM_ENCODER: dimEncoderLock(value, channel); break;
        }
    }
}

void objectUpdated(int objno)
{
    if (objno >= COMOBJ_LOCK1 && objno < COMOBJ_LOCK1 + NUM_CHANNELS)
    {
        int channel = objno - COMOBJ_LOCK1;
        int value = objectRead(objno);

        if (value != lastLock[channel])
        {
            lastLock[channel] = value;
            channelLockChanged(channel, value);
        }
    }
}

void handlePeriodic(void)
{
	int channel;

    for (channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        if (objectRead(COMOBJ_LOCK1 + channel)) // check if channel is currently locked
            return;

        params = channelParams + (channel << 2);
        const int type = channelType(channel);
        switch (type)
        {
        case CHANNEL_TYPE_SWITCH:
        	switchPeriodic(channel    );
        	switchPeriodic(channel + 8);
            break;
        case CHANNEL_TYPE_DIM:         dimPeriod(channel);    break;
        case CHANNEL_TYPE_JALO:        jaloPeriod(channel);   break;
        case CHANNEL_TYPE_DIM_ENCODER: dimEncPeriod(channel); break;

        default:
            break;
        }
    }
}

unsigned int calculateTime(int base, int factor)
{
	unsigned int result = 132;
	while (base)
	{
		result *=2;
		base--;
	}
	return result * factor;
}

void initApplication(void)
{
	int channel;
    for (channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        params = channelParams + (channel << 2);

    	timeout[channel + 0].stop();
    	timeout[channel + 8].stop();
        const int type = channelType(channel);
        // set the initial state of the lock object
		lastLock[channel] = (params [0] & 0x03) == 0x02;
        objectSetValue(COMOBJ_LOCK1      + channel, 0);

        switch (type)
        {
        case CHANNEL_TYPE_SWITCH:      switchSetup(channel);     break;
        case CHANNEL_TYPE_DIM:         dimSetup(channel);        break;
        case CHANNEL_TYPE_JALO:        jaloSetup(channel);       break;
        case CHANNEL_TYPE_DIM_ENCODER: dimEncoderSetup(channel); break;
        }
    }
}
