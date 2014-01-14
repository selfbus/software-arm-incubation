/*
 *  debounce.cpp - A debouncer.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/debounce.h>

#include <sblib/timer.h>


Debouncer::Debouncer()
:time(0)
{
}

int Debouncer::debounce(int current, unsigned int timeout)
{
    const unsigned int now = millis();

    if (last != current || !time)
    {
        time = now;
        last = current;
    }
    else if (last == current && now - (time + timeout) > 0)
    {
        time = 0;
        valid = current;
    }

    return valid;
}
