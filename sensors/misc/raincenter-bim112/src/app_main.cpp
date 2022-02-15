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

#include <string.h>
#include <sblib/eib.h>
#include <sblib/eib/user_memory.h>
#include <sblib/eib/sblib_default_objects.h>
#include "config.h"
#include "app_raincenter.h"

APP_VERSION("SBrain  ", "0", "30");

const HardwareVersion * currentVersion;

/**
 * Application setup
 */
void setup()
{
    currentVersion = &hardwareVersion[HARDWARE_ID];
    bcu.begin(MANUFACTURER, currentVersion->hardwareVersion[5], 0x28);  // we are a MDT shutter/blind actuator, version 2.8
    memcpy(userEeprom.order, currentVersion->hardwareVersion, sizeof(currentVersion->hardwareVersion));

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
