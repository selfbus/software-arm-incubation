/*
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_in.h"
//#include "debug.h"
#include <sblib/eib.h>
#include <sblib/eib/user_memory.h>
#include <sblib/eib/sblib_default_objects.h>
#include <string.h> /* for memcpy() */
#include "config.h"


const HardwareVersion * currentVersion;
/**
 * Application setup
 */
void setup()
{
    //bcu.setProgPin(PIN_PROG);
#ifndef __LPC11UXX__
    //bcu.setProgPinInverted(false);
    //bcu.setRxPin(PIO1_8);
    //bcu.setTxPin(PIO1_9);
#endif

    //debug_init();
	currentVersion = &hardwareVersion[HARDWARE_ID];
    bcu.begin(MANUFACTURER, currentVersion->deviceType, currentVersion->appVersion);

    // XXX read some ID pins to determine which version is attached

    // FIXME for new memory mapper
    memcpy(userEeprom.order, currentVersion->hardwareVersion,
            sizeof(currentVersion->hardwareVersion));
    initApplication();
}

/**
 * The application's main.
 */
void loop()
{
    int objno;
    // Handle updated communication objects
    while ((objno = nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }

    checkPeriodic();

    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}
