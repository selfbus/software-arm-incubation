/*
 *  counter.cpp -
 *
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "counter.h"
#include <sblib/eib.h>

// the following actions for individually configured channels
#define COUNT_ON_RISING_EDGE         0x0000
#define COUNT_ON_FALLING_EDGE        0x0001
#define COUNT_ON_ANY_EDGE            0x0002


Counter::Counter(unsigned int no, unsigned int longPress, unsigned int channelConfig, unsigned int busReturn, unsigned int value)
    : _Switch_ (no, longPress)
{
	modeCounter   = userEeprom [channelConfig + 0x03];
	txDiffCounter = userEeprom [channelConfig + 0x1d];

    debug_eeprom("Channel EEPROM:", channelConfig, 46);

    resetComObjNo   = 5 * no;
    counterComObjNo = 5 * no + 3;
}

void Counter::inputChanged(int value)
{
	int counter = objectRead(counterComObjNo);
	int countok = 0;
	if(modeCounter == COUNT_ON_RISING_EDGE && value)
	{
		counter++;
		countok=1;
	}
	if(modeCounter == COUNT_ON_FALLING_EDGE && !value)
	{
		counter++;
		countok=1;
	}
	if(modeCounter == COUNT_ON_ANY_EDGE)
	{
		counter++;
		countok=1;
	}
	if(countok)
	{
		if(!(counter % txDiffCounter))
		{
			objectWrite(counterComObjNo, counter);
		} else
		{
			objectSetValue(counterComObjNo, counter);
		}
	}
}

void Counter::checkPeriodic(void)
{
    if(objectRead(resetComObjNo))
    {
    	int val = 0;
    	objectSetValue(resetComObjNo, val);
    	objectWrite(counterComObjNo, val);
    }
}
