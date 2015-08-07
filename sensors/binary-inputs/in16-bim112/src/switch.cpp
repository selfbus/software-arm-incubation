/*
 *  switch.cpp - 
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "switch.h"
#include <sblib/eib.h>

/**
 * Channel actions
 */
// the following actions for individually configured channels
#define SEND_ON_RISING_EDGE         0x0000
#define TOGGLE_RISING_EDGE          0x0001
#define SEND_ON_FALLING_EDGE        0x0002
#define TOGGLE_FALLING_EDGE         0x0003
#define SEND_STATE                  0x0005
#define SEND_STATE_ON_DELAY         0x0008
#define SEND_STATE_OFF_DELAY        0x0009
#define SEND_VALUE_RISING_EDGE      0x0100
#define SEND_VALUE_FALLING_EDGE     0x0102
#define SEND_VALUE_ANY_EDGE         0x0104

// the following actions are if two channels are grouped
#define GROUP_SEND_ON               0x000C
#define GROUP_SEND_OFF              0x000D

// the following actions are for the short/long operation
#define LS_SEND_OFF                 0x00
#define LS_SEND_ON                  0x01
#define LS_TOGGLE                   0x02
#define LS_SEND_VALUE               0x03
#define LS_DO_NOTHING               0xFF

void _Switch_::setLock(unsigned int value)
{
    Channel::setLock(value);
    timeout.stop();
}

Switch::Switch(unsigned int no, unsigned int longPress, unsigned int channelConfig, unsigned int busReturn, unsigned int value)
    : _Switch_ (no, longPress)
{
    action        = userEeprom.getUInt16(channelConfig + 2);
    usage_falling = userEeprom [channelConfig + 4];
    usage_rising  = userEeprom [channelConfig + 8];
    delay         = userEeprom.getUInt16(channelConfig + 0x1E) * 1000;
    if (busReturn)
    {
        switch (action)
        {
        case TOGGLE_RISING_EDGE:
        case TOGGLE_FALLING_EDGE:
            requestObjectRead(number * 5 + 1);
            break;
        case SEND_STATE:
        case SEND_VALUE_ANY_EDGE:
            if (value)
                objectWrite(number * 5, usage_rising);
            else
                objectWrite(number * 5, usage_falling);
            break;
        default:
            break;
        }
    }
}

void Switch::inputChanged(int value)
{
    int objNo = number * 5;
    unsigned int objValue;

    if (value)
    {   // this change is a rising edge
		switch (action)
		{
		case SEND_ON_RISING_EDGE:
			objValue = usage_rising;
			break;
		case TOGGLE_RISING_EDGE:
			objValue = !objectRead(objNo+1);
			break;
		case GROUP_SEND_ON:
			objNo = (number / 2) * 10;
			objValue = 1;
			break;
		case GROUP_SEND_OFF:
			objNo = (number / 2) * 10;
			objValue = 0;
			break;
		case SEND_STATE:
			objValue = usage_rising;
			if (delay)
				timeout.start(delay);
			break;
		case SEND_STATE_ON_DELAY:
			objNo = -1; // don't send a object now
			timeout.start(delay);
			break;
		case SEND_STATE_OFF_DELAY:
			objValue = 1;
			break;
		case SEND_VALUE_RISING_EDGE:
		case SEND_VALUE_ANY_EDGE:
			objValue = usage_rising;
			break;
		default:
			objNo = -1;
		}
    }
    else
    {   // this change is a falling edge
		switch (action)
		{
		case SEND_ON_FALLING_EDGE:
			objValue = usage_falling;
			break;
		case TOGGLE_FALLING_EDGE:
			objValue = !objectRead(objNo+1);
			break;
		case SEND_STATE:
			objValue = usage_falling;
			if (delay)
				timeout.start(delay);
			break;
		case SEND_STATE_ON_DELAY:
			objValue = 0;
			break;
		case SEND_STATE_OFF_DELAY:
			objNo = -1; // don't send a object now
			timeout.start(delay);
			break;
		case SEND_VALUE_ANY_EDGE:
		case SEND_VALUE_FALLING_EDGE:
			objValue = usage_falling;
			break;
		default:
			objNo = -1;
		}
    }
    if (objNo >= 0)
    {
        objectWrite(objNo, objValue);
    }
}

void Switch::checkPeriodic(void)
{
    if(timeout.started() && timeout.expired())
    {
        unsigned int objValue;
        int objNo = number * 5;
        switch (action)
        {
        case SEND_STATE_ON_DELAY:
            objValue = 1;
            break;
        case SEND_STATE_OFF_DELAY:
            objValue = 0;
            break;
        case SEND_STATE:
            objValue = objectRead(objNo);
            timeout.start(delay);
            break;
        default:
            objNo = -1;
        }
        if (objNo >= 0)
        {
            objectWrite(objNo, objValue);
        }
    }
}

Switch2Level::Switch2Level(unsigned int no, unsigned int longPress, unsigned int channelConfig, unsigned int busReturn, unsigned int value)
    : _Switch_ (no, longPress)
{
	shortAction   = userEeprom [channelConfig + 0x14];
	longAction    = userEeprom [channelConfig + 0x17];
    usage_falling = userEeprom [channelConfig + 0x04];
    usage_rising  = userEeprom [channelConfig + 0x10];

    if (busReturn)
    {
        switch (shortAction)
        {
        case LS_TOGGLE:
            requestObjectRead(number * 5 + 1);
            break;
        default:
            break;
        }
        switch (longAction)
        {
        case LS_TOGGLE:
            requestObjectRead(number * 5 + 3);
            break;
        default:
            break;
        }
    }
}

void Switch2Level::inputChanged(int value)
{
	int objNo = number * 5;
	unsigned int objValue = -1;

	if (value)
	{   // this change is a rising edge, just start the long pressed timeout
	    // if a falling edge occurs before the timeout expires
	    // the short action will be triggered
	    // if the long press timeout expires -> the long press action will be
	    // triggered
	    timeout.start(longPressTime);
	}
	else
	{   // this change is a falling edge
		// only handle the falling edge if we don't had a long pressed
		// for the last rising edge
		if (timeout.started())
		{
			switch (shortAction)
			{
			case LS_SEND_OFF:
				objValue = 0;
				break;
			case LS_SEND_ON:
				objValue = 1;
				break;
			case LS_TOGGLE:
				objValue = !objectRead(objNo+1);
				break;
			case LS_SEND_VALUE:
				objValue = usage_falling & 0xFF;
				break;
			case LS_DO_NOTHING:
			default:
				objNo = -1;
			}
			timeout.stop();
			if (objNo >= 0)
			{
				objectWrite(objNo, objValue);
			}
		}
	}
}

void Switch2Level::checkPeriodic(void)
{
    int objNo = number * 5 + 2;
    unsigned int objValue;
    if (timeout.started() && timeout.expired())
    {
        switch (longAction)
        {
        case LS_SEND_OFF:
            objValue = 0;
            break;
        case LS_SEND_ON:
            objValue = 1;
            break;
        case LS_TOGGLE:
            objValue = !objectRead(objNo+1);
            break;
        case LS_SEND_VALUE:
            objValue = usage_rising & 0xFF;
            break;
        case LS_DO_NOTHING:
        default:
            objNo = -1;
        }
        if (objNo >= 0)
        {
            objectWrite(objNo, objValue);
        }
    }
}
