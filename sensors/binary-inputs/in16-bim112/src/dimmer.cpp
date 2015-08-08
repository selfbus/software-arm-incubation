/*
 *  dimmer.cpp -
 *
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "dimmer.h"
#include <sblib/serial.h>
#include <sblib/eib.h>

Dimmer::Dimmer(unsigned int no, unsigned int longPress, unsigned int channelConfig, unsigned int busReturn, unsigned int value)
    : _Switch_ (no, longPress)
{
	oneButtonDimmer      = userEeprom [channelConfig + 0x03]&0x03?0:1;
	upDownInverse        = userEeprom [channelConfig + 0x03]&0x01?1:0;
	stepsDownWidthDimmer = userEeprom [channelConfig + 0x04];
	stepsUpWidthDimmer   = userEeprom [channelConfig + 0x08];
	repeatDimmerTime     = userEeprom.getUInt16(channelConfig + 0x1E);
	if(!userEeprom[channelConfig + 0x20] & 0x3) {
		repeatDimmerTime = 0;
	}
	doStopFlag = 0;

	if(!oneButtonDimmer) {
		if(!(no&1)) {
			onOffComObjNo = no * 5;
			dimValComObjNo = no * 5 + 1;
		} else {
			onOffComObjNo = (no -1 ) * 5;
			dimValComObjNo = (no -1) * 5 + 1;
		}
		stateComObjNo = -1;
	} else {
		if(oneButtonDimmer) {
			onOffComObjNo = no * 5;
			dimValComObjNo = no * 5 + 1;
			stateComObjNo = no * 5 + 2;
		}
	}
	if (busReturn && oneButtonDimmer)
	{
		requestObjectRead(stateComObjNo);
	}
}

void Dimmer::inputChanged(int value)
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
		if(doStopFlag) {
			int objVal = 0;
			doStopFlag = 0;
			objectWrite(dimValComObjNo, objVal);
    		upDownInverse = !upDownInverse;
		} else if (timeout.started())
		{
			unsigned int state = !upDownInverse;
			if(oneButtonDimmer) { // in one button mode use the inverse direction com obj value
				state = !objectRead(stateComObjNo);
				objectSetValue(stateComObjNo, state);
			}
			objectWrite(onOffComObjNo, state);
		}
		timeout.stop();
	}
}

void Dimmer::checkPeriodic(void)
{
    if (timeout.started() && timeout.expired())
    {
    	int val=((number&1)^upDownInverse)?stepsDownWidthDimmer:stepsUpWidthDimmer;
    	if(oneButtonDimmer) {
    		if(repeatDimmerTime) {
    			timeout.start(repeatDimmerTime);
    		}
    	}
		objectWrite(dimValComObjNo, val);
		doStopFlag = 1;
    }
}
