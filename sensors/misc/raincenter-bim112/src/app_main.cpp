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

// create APP_VERSION, its used in the bus updater magic string is !AVP!@:
// from Rauchmelder-bcu1 (app_main.cpp):
volatile const char __attribute__((used)) APP_VERSION[20] = "!AVP!@:SBrain  0.30";
// disable optimization seems to be the only way to ensure that this is not being removed by the linker
// to keep the variable, we need to declare a function that uses it
// alternatively, the link script may be modified by adding KEEP to the section
volatile const char * __attribute__((optimize("O0"))) getAppVersion()
{
    return APP_VERSION;
}

const HardwareVersion * currentVersion;

/**
 * Application setup
 */
void setup()
{
    volatile const char * v = getAppVersion();      // Ensure APP ID is not removed by linker (its used in the bus updater)
    v++;                                            // just to avoid compiler warning of unused variable
    currentVersion = &hardwareVersion[HARDWARE_ID];
    bcu.begin(MANUFACTURER, currentVersion->hardwareVersion[5], 0x28);  // we are a MDT shutter/blind actuator, version 2.8
    memcpy(userEeprom.order(), currentVersion->hardwareVersion, sizeof(currentVersion->hardwareVersion));

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
