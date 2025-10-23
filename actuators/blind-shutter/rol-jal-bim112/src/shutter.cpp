/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <shutter.h>

Shutter::Shutter(uint8_t newNumber, uint32_t newAddress, uint16_t newPosition)
  : Channel(newNumber, newAddress, newPosition)
{
    if (bcu.userEeprom->getUInt8(newAddress + 65) & 0x40)
        shortTime = bcu.userEeprom->getUInt16(newAddress + 6);
}
