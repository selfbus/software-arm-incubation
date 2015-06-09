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

Shutter::Shutter(unsigned int number, unsigned int address)
  : Channel(number, address)
{
    if (userEeprom.getUInt8(address +   65) & 0x02)
        shortTime = userEeprom.getUInt16(address +   6);
}


