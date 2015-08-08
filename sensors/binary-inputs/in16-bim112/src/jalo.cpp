/*
 *  jalo.cpp -
 *
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "jalo.h"
#include <sblib/eib.h>

Jalo::Jalo(unsigned int no, unsigned int longPress, unsigned int channelConfig, unsigned int busReturn, unsigned int value)
    : _Switch_ (no, longPress)
{
	oneButtonShutter = userEeprom [channelConfig + 0x03]&0x04?1:0;
	shortLongInverse = userEeprom [channelConfig + 0x03]&0x02?1:0;
	upDownInverse    = userEeprom [channelConfig + 0x03]&0x01?1:0;

	if(!oneButtonShutter) {
		if(!(no&1)) {
			upDownComObjNo = no * 5;
			stopComObjNo = no * 5 + 1;
		} else {
			upDownComObjNo = (no -1 ) * 5;
			stopComObjNo = (no -1) * 5 + 1;
		}
		directionComObjNo = -1;
	} else {
		if(oneButtonShutter) {
			upDownComObjNo = no * 5;
			stopComObjNo = no * 5 + 1;
			directionComObjNo = no * 5 + 2;
		}
	}
	if (busReturn && oneButtonShutter)
	{
		requestObjectRead(directionComObjNo);
	}
}

void Jalo::inputChanged(int value)
{
	if (value)
	{   // this change is a rising edge, just start the long pressed timeout
	    // if a falling edge occurs before the timeout expires
	    // the short action will be triggered
	    // if the long press timeout expires -> the long press action will be
	    // triggered
	    timeout.start(longPressTime);
	}
	else
	{   // this change is a falling edge
		// only handle the falling edge if we don't had a long pressed
		// for the last rising edge
		if (timeout.started())
		{
			unsigned int direction = upDownInverse; //use inverse value from parameters
			if(oneButtonShutter) { // in one button mode use the inverse direction com obj value
				direction = !objectRead(directionComObjNo);
			}
			if(!shortLongInverse) {
				objectWrite(stopComObjNo, direction);
			} else {
				if(oneButtonShutter) {
					objectSetValue(directionComObjNo, direction);
				}
				objectWrite(upDownComObjNo, direction);
			}
		}
		timeout.stop();
	}
}

void Jalo::checkPeriodic(void)
{
    if (timeout.started() && timeout.expired())
    {
		unsigned int direction = upDownInverse; //use inverse value from parameters
		if(oneButtonShutter) { // in one button mode use the inverse direction com obj value
			direction = !objectRead(directionComObjNo);
		}
		if(shortLongInverse) {
			objectWrite(stopComObjNo, direction);
		} else {
			if(oneButtonShutter) {
				objectSetValue(directionComObjNo, direction);
			}
			objectWrite(upDownComObjNo, direction);
		}
    }
}
