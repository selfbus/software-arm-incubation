/*
 *  digital_pin_shift.cpp - Functions for bit shifting digital I/O
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include <sblib/digital_pin.h>

byte shiftIn(int dataPin, int clockPin, BitOrder bitOrder)
{
    int value = 0;
    int i;

    for (i = 0; i < 8; ++i)
    {
        digitalWrite(clockPin, 1);

        if (bitOrder == LSBFIRST)
            value |= digitalRead(dataPin) << i;
        else value |= digitalRead(dataPin) << (7 - i);

        digitalWrite(clockPin, 0);
    }
    return value;
}

void shiftOut(int dataPin, int clockPin, BitOrder bitOrder, byte val)
{
    int i, value = val;

    for (i = 0; i < 8; i++)
    {
        if (bitOrder == LSBFIRST)
            digitalWrite(dataPin, value & (1 << i));
        else digitalWrite(dataPin, value & (1 << (7 - i)));

        digitalWrite(clockPin, 1);
        digitalWrite(clockPin, 0);
    }
}
