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
#include <sblib/timeout.h>

// The parameters of the current channel (4 bytes)
const byte* params;
static Timeout timeout[NUM_CHANNELS * 2];
static unsigned int delayTime[NUM_CHANNELS * 2];
static int lastLock[NUM_CHANNELS];
static union
{
	struct
	{
		unsigned char started;
	} dim;
	struct
	{
		bool        first;
		signed char step;
		signed int  value;
	} dimenc;
} channelData[NUM_CHANNELS];


#define DIM_STEP_MASK      0x07
#define DIM_DIRECTION_MASK 0x08

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

/*
 * The value of an input channel of type "switch" changed.
 *
 * @param channel - the input channel (0..7)
 * @param pinValue - the current value of the input pin (0 or 1)
 */
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

void dimChannelChanged(int channel, int pinValue)
{
	int method          = (params [0] & 0x70) >> 4;
    if (pinValue) // raising edge
    {   // start the time between switch and dim
    	timeout[channel].start(delayTime[channel]);
    	unsigned int value  = !objectRead(channel);
    	if (method == DIMMER_TYPE_TWO_HAND_LIGHTER_ON)
    		value = 1;
    	if (method == DIMMER_TYPE_TWO_HAND_DARKER_OFF)
		    value = 0;
    	// send telegram on switching object
    	objectWrite(COMOBJ_PRIMARY1 + channel, value);
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
    	timeout[channel    ].stop();
    	timeout[channel + 8].stop();
    	channelData[channel].dim.started = 0;
    }
}

void jaloChannelChanged(int channel, int pinValue)
{
	unsigned int ls     = params [3] & 0x08;
	unsigned int value  = objectRead(COMOBJ_SECONDARY1 + channel);
	unsigned int action = (params [3] & 0xF0) >> 4;
	if (pinValue)
	{   // rising edge
		switch (action)
		{
		case 1 : value = 1;      break;
		case 2 : value = 0;      break;
		case 3 : value = !value; break;
		}
		if (! ls)
		{   // sls concept -> send short telegram
			objectWrite(COMOBJ_PRIMARY1   + channel, value);
			// start the time between short and log
			timeout[channel + 8].start(delayTime[channel + 8]);
		}
		else
		{   // ls concept -> send long telegram
			objectWrite(COMOBJ_SECONDARY1 + channel, value);
			// start the blade changing time
		    timeout[channel].start(delayTime[channel]);
		}
	}
	else
	{   // falling edge
		if (timeout[channel].started() && !timeout[channel].expired())
		{
			objectWrite(COMOBJ_PRIMARY1   + channel, value);
		}
		if (timeout[channel + 8].started() && !timeout[channel + 8].expired())
		{   // falling edge inside short-long time -> no further action
		}
		// in any case, after a falling edge, stop all timers
		timeout[channel    ].stop();
		timeout[channel + 8].stop();
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


void channelLockChanged(int channel, unsigned int value)
{
    params = channelParams + (channel << 2);
    int lockPolarity = params [0] & 0x03;
    int lockAction;
    if (lockPolarity)
    {
        const int type = channelType(channel);
    	if (lockPolarity == 2) value = !value; // inverted lock logic
        switch (value)
        {
        case 1:   // start of locking
        	switch (type)
        	{
            case CHANNEL_TYPE_SWITCH:
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
            	break;

			case CHANNEL_TYPE_DIM:
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
				break;

			case CHANNEL_TYPE_JALO:
				lockAction = (params [0] & 0x30) >> 4;
				switch(lockAction)
				{
				case EE_JALO_LOCK_SET_DOWN:
				case EE_JALO_LOCK_SET_UP:
					value = lockAction == EE_JALO_LOCK_SET_UP ? 1 : 0;
					objectWrite(COMOBJ_SECONDARY1 + channel, value);
					break;
				case EE_JALO_LOCK_TOGGLE:
					value = !objectRead(COMOBJ_SECONDARY1 + channel);
					objectWrite(COMOBJ_SECONDARY1 + channel, value);
					break;
				case EE_JALO_LOCK_NO_REACTION:
				default:
					break;
				}
				break;

			case CHANNEL_TYPE_DIM_ENCODER:
				dimEncAction(channel, params [1] & 0x03);
			    break;
        	}
        	break;
        case 0:   // end of locking
        	switch (type)
        	{
            case CHANNEL_TYPE_SWITCH:
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
            	break;

			case CHANNEL_TYPE_DIM:
				if (params [0] & 0x80)
				{
					objectWrite(COMOBJ_PRIMARY1   + channel, (unsigned int) 0);
				}
				break;

			case CHANNEL_TYPE_JALO:
				lockAction = (params [0] & 0x0C) >> 2;
				switch(lockAction)
				{
				case EE_JALO_LOCK_SET_DOWN:
				case EE_JALO_LOCK_SET_UP:
					value = lockAction == EE_JALO_LOCK_SET_UP ? 1 : 0;
					objectWrite(COMOBJ_SECONDARY1 + channel, value);
					break;
				case EE_JALO_LOCK_TOGGLE:
					value = !objectRead(COMOBJ_SECONDARY1 + channel);
					objectWrite(COMOBJ_SECONDARY1 + channel, value);
					break;
				case EE_JALO_LOCK_NO_REACTION:
				default:
					break;
				}
				break;

			case CHANNEL_TYPE_DIM_ENCODER:
				dimEncAction(channel, (params [1] & 0x30) >> 4 );
	            break;
        	}
        	break;
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

static void _switchPeriodic (int channel)
{
	if (timeout[channel].expired())
	{
		int cyclicCfg = (params [0] & 0x0C) >> 2; // get the cyclic configuration
		int value     = objectRead(channel);
		if ( value && (cyclicCfg & 0x01)) // send cyclic telegram if value is ON
			setObjectFlags(channel, COMFLAG_TRANSREQ);
		if (!value && (cyclicCfg & 0x02)) // send cyclic telegram if value is OFF
			setObjectFlags(channel, COMFLAG_TRANSREQ);
		timeout[channel].start(delayTime[channel]);
	}
}

static void _dimPeriod(int channel)
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

static void _jaloPeriod(int channel)
{
	unsigned int ls = params [3] & 0x08;
	unsigned int value;
    if (! ls)
    {   // short-long-short concept
    	if (timeout[channel + 8].expired())
    	{
    		value = objectRead(COMOBJ_PRIMARY1 + channel);
    		objectWrite(COMOBJ_SECONDARY1 + channel, value);
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

static void _dimEncPeriod(int channel)
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
        	_switchPeriodic(channel    );
        	_switchPeriodic(channel + 8);
            break;
        case CHANNEL_TYPE_DIM:         _dimPeriod(channel);    break;
        case CHANNEL_TYPE_JALO:        _jaloPeriod(channel);   break;
        case CHANNEL_TYPE_DIM_ENCODER: _dimEncPeriod(channel); break;

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

void initApplication(void)
{
	int channel;

    for (channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        params = channelParams + (channel << 2);

    	timeout[channel + 0].stop();
    	timeout[channel + 8].stop();
        const int type = channelType(channel);
        unsigned int value;
        int busReturnCfg;
        // set the initial state of the lock object
		lastLock[channel] = (params [0] & 0x03) == 0x02;
        objectSetValue(COMOBJ_LOCK1      + channel, 0);

        switch (type)
        {
        case CHANNEL_TYPE_SWITCH:
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
        	break;
        case CHANNEL_TYPE_DIM:
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
        	break;
        case CHANNEL_TYPE_JALO:
        	// Calculate blade change time
        	value = userEeprom[EE_CHANNEL_TIMING_PARAMS_BASE + ((channel + 1 + 8) >> 1)];
        	if (! (channel & 0x01)) value >>= 4;
        	else                    value  &= 0x0F;

        	delayTime[channel    ] = calculateTime(value, params [2] & 0x7F);
        	if (! (params[3] & 0x08)) // Short Long Short
        	{
            	// Calculate time between short and long push
            	value = userEeprom[EE_CHANNEL_TIMING_PARAMS_BASE + ((channel + 1) >> 1)];
            	if (! (channel & 0x01)) value >>= 4;
            	else                    value  &= 0x0F;

            	delayTime[channel + 8] = calculateTime(value, params [1] & 0x7F);
        	}
        	else
        		delayTime[channel + 8] = 0;
        	// handle bus return
        	switch ((params [0] & 0xC0) >> 6)
        	{
        	case EE_JALO_BUS_RETURN_DOWN: objectWrite(COMOBJ_SECONDARY1 + channel, (unsigned int) 0); break;
        	case EE_JALO_BUS_RETURN_UP:   objectWrite(COMOBJ_SECONDARY1 + channel, (unsigned int) 1); break;
        	case EE_JALO_BUS_RETURN_NO_ACTION:
        	default:
        	    objectSetValue(COMOBJ_SECONDARY1 + channel, 0);
        	    break;
        	}
        	objectSetValue(COMOBJ_PRIMARY1 + channel, 0);
        	break;
        case CHANNEL_TYPE_DIM_ENCODER:
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
        	break;
        }
    }
}
