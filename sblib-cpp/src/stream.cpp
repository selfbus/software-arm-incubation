/*
 *  stream.cpp - Base class for character-based streams.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/stream.h>

#include <sblib/math.h>
#include <sblib/timer.h>


int Stream::timedRead()
{
    int ch, start = millis();

    do
    {
        ch = read();
        if (ch >= 0) return ch;
    }
    while (elapsed(start) < timeout);

    return -1;
}

#ifdef PEEK_NOT_IMPLEMENTED
int Stream::timedPeek()
{
    int ch, start = millis();

    do
    {
        ch = peek();
        if (ch >= 0) return ch;
    }
    while (elapsed(start) < timeout);

    return -1;
}
#endif

