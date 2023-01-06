/*
 *  channel.cpp - 
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "channel.h"
#include <sblib/eibMASK0701.h>

extern MASK0701 bcu;

Channel::Channel()
: pwm(0)
{
}

// XXX implement the curves
void Channel::setValue(unsigned char value)
{
    uint8_t eepromContent = bcu.userEeprom->getUInt8(0x4762);
    if (eepromContent == 0)
    {   // quadratic curve
        pwm = value;
    }
    else if (eepromContent == 2)
    {   // semi-logarithmic curve
        pwm = value;
    }
    else if (eepromContent == 7)
    {   // linear curve
        pwm = value;
    }
}

