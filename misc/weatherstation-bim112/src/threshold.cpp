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
, changeLowerLimit(-1)
, changeUpperLimit(-1)
, blockObjNo(-1)
{
    cycleTimeout.start(cycleTime);
}

unsigned int Threshold::periodic(unsigned int value)
{
    unsigned int result = 0;

    if (objNumber < 0) // this threshold is not active
        return result;
    int objNo = -1;
    if (value > upperLimit)
    {   // the current value is above the limit
        if (state & UPPER_LIMIT_TIME_ACTIVE)
        {   // we have already started the timeout monitoring for the upper limit
            if (limitTimeout.expired())
            {   // the value was above the limit for the required time
                state = IN_UPPER_LIMIT;
                result = IN_UPPER_LIMIT;
                if (sendLimitExceeded)
                    // mark a request for send
                    objNo = objNumber;
            }
        }
        else
        {
            state = UPPER_LIMIT_TIME_ACTIVE;
            limitTimeout.start (upperLimitTime);
        }
    }
    if (value < lowerLimit)
    {   // the current value is below the limit
        if (state & LOWER_LIMIT_TIME_ACTIVE)
        {   // we have already started the timeout monitoring for the lower limit
            if (limitTimeout.expired())
            {   // the value was below the limit for the required time
                state = IN_LOWER_LIMIT;
                result = IN_LOWER_LIMIT;
                if (sendLowerDeviation)
                    // mark a request for send
                    objNo = objNumber;
            }
        }
        else
        {
            state = LOWER_LIMIT_TIME_ACTIVE;
            limitTimeout.start (lowerLimitTime);
        }
    }
    if (cycleTimeout.expired ())
    {
        cycleTimeout.start(cycleTime);
        objNo = objNumber;
    }
    if (objNo >= 0)
    {   // we should send the value
        if (state & IN_LOWER_LIMIT)
            objectWrite(objNumber, sendLowerDeviation == 1 ? 1 : 0);
        if (state & IN_UPPER_LIMIT)
            objectWrite(objNumber, sendLimitExceeded == 1 ? 1 : 0);
    }
    return result;
}
