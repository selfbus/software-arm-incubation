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

/*
 * Raincenter objects for testing
 * JAL-410.01 Kanal A Rolladen
 * objects
 * 13 = TapWaterRefill (on/off)
 * 17 = TapWaterRefill Status (on/off)
 * 20 = Calibrated Fill Level in m3 * 10 (43 means 4.3m³)
 *
 *
 *
 *
 */

#include <sblib/eib.h>
#include <sblib/eib/user_memory.h>
#include <sblib/eib/sblib_default_objects.h>
#include <string.h> /* for memcpy() */
#include "config.h"
#include "app_raincenter.h"



extern "C" const char APP_VERSION[13] = "BS4.70  0.9";

const char * getAppVersion()
{
    return APP_VERSION;
}

const HardwareVersion * currentVersion;
/**
 * Application setup
 */
void setup()
{
    currentVersion = &hardwareVersion[HARDWARE_ID];
    volatile char v = getAppVersion()[0];
    v++;
    bcu.begin(MANUFACTURER, currentVersion->hardwareVersion[5], 0x28);  // we are a MDT shutter/blind actuator, version 2.8
    memcpy(userEeprom.order, currentVersion->hardwareVersion, sizeof(currentVersion->hardwareVersion));


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
