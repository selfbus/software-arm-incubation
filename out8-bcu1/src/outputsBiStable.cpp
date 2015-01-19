/*
 *  outputsBiStable.cpp - Handle the update of the port pins in accordance to the
 *                need of the application
 *
 *  Copyright (C) 2014-2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "outputsBiStable.h"

void OutputsBiStable::updateOutputs(void)
{
    unsigned int mask = 0x01;
    unsigned int i;
    unsigned int state = _relayState ^ _inverted;
	unsigned int data = 0;

    for (i = 0; i < NO_OF_CHANNELS; i++, mask <<= 1)
    {
    	data <<= 2;
    	if (state & mask)
    		 data |= 0b01;
    	else data |= 0b10;
    }
}

#ifdef BI_STABLE
OutputsBiStable relays;
#endif


