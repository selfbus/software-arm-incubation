/*
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_in.h"
#include "debug.h"
#include "config.h"

APP_VERSION("SBin16  ", "1", "11")

const HardwareVersion * currentVersion;

/**
 * Application setup
 */
BcuBase* setup()
{
    //bcu.setProgPin(PIN_PROG);
#ifndef __LPC11UXX__
    //bcu.setProgPinInverted(false);
    //bcu.setRxPin(PIO1_8);
    //bcu.setTxPin(PIO1_9);
#endif

    //debug_init();
	currentVersion = &hardwareVersion[HARDWARE_ID];
	bcu.setHardwareType(currentVersion->hardwareVersion, sizeof(currentVersion->hardwareVersion));
	bcu.begin(MANUFACTURER, currentVersion->deviceType, currentVersion->appVersion);

    // XXX read some ID pins to determine which version is attached

    initApplication();
    return (&bcu);
}

/**
 * The application's main.
 */
void loop()
{
    int objno;
    // Handle updated communication objects
    while ((objno = bcu.comObjects->nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }

    checkPeriodic();

    // Sleep up to 1 millisecond if there is nothing to do
    if (bcu.bus->idle())
        waitForInterrupt();
}

/**
 * The processing loop while no KNX-application is loaded
 */
void loop_noapp()
{

}
