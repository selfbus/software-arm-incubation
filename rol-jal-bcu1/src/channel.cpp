/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "channel.h"
#include <sblib/digital_pin.h>
#include <sblib/timer.h>

const int outputPins[NO_OF_OUTPUTS] =
    { PIO2_2, PIO0_7, PIO2_10, PIO2_9, PIO0_2, PIO0_8, PIO0_9, PIO2_11 };

Channel::Channel(unsigned int number)
  : number(number)
  , openTime(0)
  , closeTime(0)
  , position(0)
  , status(STOP)
{
    for (unsigned int i = 0;i < NO_OF_OUTPUTS; i++)
    {
        pinMode(outputPins [i], OUTPUT);
        digitalWrite(outputPins [i], 0);
    }
}

unsigned int Channel::timeToPercentage(unsigned int startTime, unsigned int maxTime)
{
    unsigned int diff = millis() - startTime;
    return (diff * 256) / maxTime;
}
