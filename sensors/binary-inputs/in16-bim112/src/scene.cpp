/*
 *  scene.cpp -
 *
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "scene.h"
#include <sblib/serial.h>
#include <sblib/eib.h>

Scene::Scene(unsigned int no, unsigned int longPress, unsigned int channelConfig, unsigned int busReturn, unsigned int value)
    : _Switch_ (no, longPress)
{
	saveScene   = userEeprom [channelConfig + 0x03]&0x01;
	numberScene = userEeprom [channelConfig + 0x04];

    sceneComObjNo = 5 * no + 2;
}

void Scene::inputChanged(int value)
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
			objectWrite(sceneComObjNo, numberScene);
		}
		timeout.stop();
	}
}

void Scene::checkPeriodic(void)
{
    if (timeout.started() && timeout.expired())
    {
    	if(saveScene) {
    		objectWrite(sceneComObjNo, numberScene|0x80);
    	}
    }
}
