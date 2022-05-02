/*
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <Appl.h>
#include "config.h"

#ifdef DEBUG
#   include <sblib/serial.h>  //debugging only
#endif

APP_VERSION("SBdim4  ", "1", "00")

const HardwareVersion * currentVersion;
unsigned int pwmmax;		//  je nach Parametrierung PWM_MAX_600 oder PWM_MAX_1000

/**
 * Application setup
 */
BcuBase* setup()
{
#ifdef DEBUG
	serial.setTxPin(PIO3_0);
	serial.setRxPin(PIO3_1);
	serial.begin(115200);
	serial.println("dim4-bim112 started");
#endif
	currentVersion = &hardwareVersion[HARDWARE_ID];
	bcu.begin(MANUFACTURER, currentVersion->deviceType, currentVersion->appVersion);
    bcu.setHardwareType(currentVersion->hardwareVersion, sizeof(currentVersion->hardwareVersion));
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
