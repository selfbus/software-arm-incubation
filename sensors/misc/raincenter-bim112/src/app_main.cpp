/*
 *  app_main.cpp
 *
 *  Created on: 11.09.2020
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "config.h"
#include "app_raincenter.h"

APP_VERSION("SBr_cent", "0", "41");

/**
 * Application setup
 */
BcuBase* setup()
{
    bcu.begin(MANUFACTURER, DEVICETYPE, APPVERSION);
    bcu.setHardwareType(&hardwareVersion[0], sizeof(hardwareVersion));
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

void loop_noapp()
{

}
