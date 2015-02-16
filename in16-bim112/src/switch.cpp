/*
 *  switch.cpp - 
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
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

#define EE_WORD(base, offset) (* (word *) (base + offset))
#define EE_BYTE(base, offset) (* (byte *) (base + offset))

Switch::Switch(unsigned int no, word config, byte * params) : Channel (no)
{
    shortLongOperation  = 0;
    action              = EE_WORD(params, 2);
    usage_falling       = EE_WORD(params, 4);
    usage_rising        = EE_WORD(params, 8);
    delay               = EE_WORD(params, 0x1E) * 1000;
    if (config == 0x100)
    {   // setup channel for short/long press operation
        unsigned int shortAction = EE_BYTE(params, 0x1e);
        unsigned int longAction  = EE_BYTE(params, 0x17);
        shortLongOperation       = shortAction | (longAction << 8);
        usage_rising             = EE_WORD(params, 0x10);
        action                   = 0xFF;
    }
}

void Switch::inputChanged(int value, int longPress)
{
    int objNo = number * 5;
    unsigned int objValue;

    if (value)
    {   // this change is a rising edge
        if (shortLongOperation and longPress)
        {   // set mask to 0 to ignore the next short press falling edge
            action = 0;
            objNo += 2;
            switch (shortLongOperation >> 8)
            {
            case LS_SEND_OFF:
                objValue = 0;
                break;
            case LS_SEND_ON:
                objValue = 1;
                break;
            case LS_TOGGLE:
                objValue = !objectRead(number);
                break;
            case LS_SEND_VALUE:
                objValue = usage_rising & 0xFF;
                break;
            case LS_DO_NOTHING:
            default:
                objNo = -1;
            }
        }
        else
        {
            switch (action)
            {
            case SEND_ON_RISING_EDGE:
                objValue = usage_rising & 0xFF;
                break;
            case TOGGLE_RISING_EDGE:
                objValue = !objectRead(number);
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
                objValue = usage_rising & 0xFF;
                break;
            default:
                objNo = -1;
            }
        }
    }
    else
    {   // this change is a falling edge
        if (shortLongOperation and !longPress)
        {   // handle the short press on the falling edge
            // mask the configured action with `action` which will be set to 0
            // if a long press has already be handled
            switch (shortLongOperation & action)
            {
            case LS_SEND_OFF:
                objValue = 0;
                break;
            case LS_SEND_ON:
                objValue = 1;
                break;
            case LS_TOGGLE:
                objValue = !objectRead(number);
                break;
            case LS_SEND_VALUE:
                objValue = usage_falling & 0xFF;
                break;
            case LS_DO_NOTHING:
            default:
                objNo = -1;
            }
            // reset the mask to allow the next short press to be handled
            action = 0xFF;
        }
        else
        {
            switch (action)
            {
            case SEND_ON_FALLING_EDGE:
                objValue = usage_falling & 0xFF;
            case TOGGLE_FALLING_EDGE:
                objValue = !objectRead(number);
                break;
            case SEND_STATE:
                objValue = usage_falling;
                if (delay)
                    timeout.start(delay);
                break;
            case SEND_STATE_ON_DELAY:
                objValue = 0;
            case SEND_STATE_OFF_DELAY:
                objNo = -1; // don't send a object now
                timeout.start(delay);
                break;
            case SEND_VALUE_ANY_EDGE:
            case SEND_VALUE_FALLING_EDGE:
                objValue = usage_falling & 0xFF;
                break;
            default:
                objNo = -1;
            }
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
        unsigned int objNo = number;
        switch (action)
        {
        case SEND_STATE_ON_DELAY:
            objValue = 1;
            break;
        case SEND_STATE_OFF_DELAY:
            objValue = 0;
            break;
        case SEND_STATE:
            objValue = objectRead(number);
            timeout.start(delay);
            break;
        }
        if (objNo >= 0)
        {
            objectWrite(objNo, objValue);
        }
    }
}

void Switch::setLock(unsigned int value)
{
    Channel::setLock(value);
    timeout.stop();
}
