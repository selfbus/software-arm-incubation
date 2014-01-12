/*
 *  telegram.h - An EIB bus telegram.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/telegram.h>

#include <sblib/print.h>


void Telegram::receiver(int addr, bool isGroup)
{
    data[3] = addr >> 8;
    data[4] = addr;

    if (isGroup) data[3] |= 0x80;
    else data[3] &= ~0x80;
}

int Telegram::printTo(Print& out) const
{
    int count = length() + 7;
    if (count > TELEGRAM_SIZE)
        count = TELEGRAM_SIZE;

    int wlen = 0;
    for (int i = 0; i < count; ++i)
    {
        if (i) wlen += out.print(' ');
        wlen += out.print(data[i], HEX, 2);
    }

    return wlen;
}
