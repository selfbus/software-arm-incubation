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

//#include "debug.h"
#include <sblib/eib.h>
#include <sblib/eib/user_memory.h>
#include <sblib/eib/sblib_default_objects.h>
#include <string.h> /* for memcpy() */
#include "config.h"
#include "app_raincenter.h"

// const HardwareVersion * currentVersion;
/**
 * Application setup
 */
void setup()
{
    //debug_init();
    /*
    currentVersion = &hardwareVersion[HARDWARE_ID];
    bcu.begin(MANUFACTURER, currentVersion->deviceType, currentVersion->appVersion);


    // FIXME for new memory mapper
    memcpy(userEeprom.order, currentVersion->hardwareVersion,
            sizeof(currentVersion->hardwareVersion));
    */

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

/*
 * The main processing loop while no app is loaded.
 */
void loop_noapp()
{
    checkPeriodic();

    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}
