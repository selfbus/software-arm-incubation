/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <shutter.h>
#include <sblib/digital_pin.h>
#include <sblib/timer.h>

Shutter::Shutter(unsigned int number)
  : Channel(number)
{
}

void Shutter::stop(void)
{
    digitalWrite(outputPins[number * 2 + 0], 0);
    digitalWrite(outputPins[number * 2 + 1], 0);
    status = STOP;
}

void Shutter::startUp(void)
{
    status = UP;
    digitalWrite(outputPins[number * 2 + 1], 0); // FIXME pause between possible direction change
    digitalWrite(outputPins[number * 2 + 0], 1);
    startTime = millis();
    startPosition = position;
}

void Shutter::startDown(void)
{
    status = DOWN;
    digitalWrite(outputPins[number * 2 + 0], 0); // FIXME pause between possible direction change
    digitalWrite(outputPins[number * 2 + 1], 1);
    startTime = millis();
    startPosition = position;
}

void Shutter::periodic(void)
{
    switch (status)
    {
    case UP:
        position = startPosition - timeToPercentage(startTime, openTime);
        break;
    case DOWN:
        position = startPosition + timeToPercentage(startTime, closeTime);
        break;
    case STOP:
    default:
        break;
    }
    if (position <= 0)
    {
        position = 0;
        stop();
    }
    if (position >= 255)
    {
        position = 255;
        stop();
    }
}



