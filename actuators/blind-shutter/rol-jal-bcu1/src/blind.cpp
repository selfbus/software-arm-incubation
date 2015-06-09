/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <blind.h>
#include <sblib/digital_pin.h>
#include <sblib/timer.h>

Blind::Blind(unsigned int nummber)
  : Channel(number)
  , slatTime(0)
  , slatPosition(0)
{
}

void Blind::stop(void)
{
    digitalWrite(outputPins[number * 2 + 0], 0);
    digitalWrite(outputPins[number * 2 + 1], 0);
    status = STOP;
}

void Blind::startUp(void)
{
    status = SLAT_OPEN;
    digitalWrite(outputPins[number * 2 + 0], 1);
    startTime = millis();
    slatStartPosition = slatPosition;
}

void Blind::startDown(void)
{
    status = SLAT_CLOSE;
    digitalWrite(outputPins[number * 2 + 1], 1);
    startTime = millis();
    slatStartPosition = slatPosition;
}

void Blind::periodic(void)
{
    switch (status)
    {
    case SLAT_OPEN:
        slatPosition = slatStartPosition - timeToPercentage(startTime, slatTime);
        if (slatPosition <= 0)
        {
            slatPosition = 0;
            status = UP;
            startTime = millis();
        }
        break;
    case SLAT_CLOSE:
        slatPosition = slatStartPosition + timeToPercentage(startTime, slatTime);
        if (slatPosition >= 255)
        {
            slatPosition = 255;
            status = DOWN;
            startTime = millis();
        }
        break;
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
