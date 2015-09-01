/*
 *  threshold.cpp - The class defining the threshold behavior
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "threshold.h"
#include <sblib/eib.h>

Threshold::Threshold()
: objNumber(-1)
, immediateObjNumber(-1)
, changeLowerLimit(-1)
, changeUpperLimit(-1)
, blockObjNo(-1)
{
    cycleTimeout.start(cycleTime);
}

void Threshold::periodic(unsigned int value)
{
    if (objNumber < 0) // this threshold is not active
        return;
    bool sendRequested = false;
    state    &= ~LIMIT_JUST_ENGAGED;
    if ((value > upperLimit) && !(state & IN_UPPER_LIMIT))
    {   // the current value is above the limit
        if (!upperLimitTime || (state & UPPER_LIMIT_TIME_ACTIVE))
        {   // we have already started the timeout monitoring for the upper limit
            if (limitTimeout.expired())
            {   // the value was above the limit for the required time
                state = IN_UPPER_LIMIT | LIMIT_JUST_ENGAGED;
                if (sendLimitExceeded)
                    sendRequested = true;
            }
        }
        else
        {
            state = (state & ~LOWER_LIMIT_TIME_ACTIVE) | UPPER_LIMIT_TIME_ACTIVE;
            limitTimeout.start (upperLimitTime);
            if (immediateObjNumber >= 0)
                objectWrite(immediateObjNumber & 0xFF, immediateObjNumber > 255 ? 0 : 1);
        }
    }
    else if ((value < lowerLimit) && !(state & IN_LOWER_LIMIT))
    {   // the current value is below the limit
        if (!lowerLimitTime || (state & LOWER_LIMIT_TIME_ACTIVE))
        {   // we have already started the timeout monitoring for the lower limit
            if (limitTimeout.expired())
            {   // the value was below the limit for the required time
                state = IN_LOWER_LIMIT | LIMIT_JUST_ENGAGED;;
                if (sendLowerDeviation)
                    sendRequested = true;
            }
        }
        else
        {
            state = (state & ~UPPER_LIMIT_TIME_ACTIVE) | LOWER_LIMIT_TIME_ACTIVE;
            limitTimeout.start (lowerLimitTime);
            if (immediateObjNumber >= 0)
                objectWrite(immediateObjNumber & 0xFF, immediateObjNumber > 255 ? 1 : 0);
        }
    }
    else
    {   // stop the timeout and the timer running flags
        limitTimeout.stop ();
        state &= ~(LOWER_LIMIT_TIME_ACTIVE | UPPER_LIMIT_TIME_ACTIVE);
    }

    if (cycleTimeout.expired ())
        sendRequested = true;

    if (sendRequested)
    {   // we should send the value
        if (state & IN_LOWER_LIMIT)
            objectWrite(objNumber, sendLowerDeviation == 1 ? 1 : 0);
        if (state & IN_UPPER_LIMIT)
            objectWrite(objNumber, sendLimitExceeded == 1 ? 1 : 0);
        // we sent an update -> restart the cyclic timer
        cycleTimeout.start(cycleTime);
    }
}
