/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <shutter.h>

Shutter::Shutter(unsigned int number, unsigned int address, short position)
  : Channel(number, address, position)
{
    if (bcu.userEeprom->getUInt8(address +   65) & 0x40)
        shortTime = bcu.userEeprom->getUInt16(address +   6);
}


