/*
 *  app_main.cpp - The application's main.
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <config.h>
#include <sblib/eib.h>
#include <sblib/eib/sblib_default_objects.h>
#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>
#include "app-rol-jal.h"
#include <string.h>

#ifdef HAND_ACTUATION
#include "hand_actuation.h"
#endif

extern "C" const char APP_VERSION[13] = "BS4.70  0.9";

const char * getAppVersion()
{
    return APP_VERSION;
}


// Hardware version. Must match the product_serial_number in the VD's table hw_product
const HardwareVersion hardwareVersion[] =
{ {4, 0x4578, { 0, 0, 0, 0, 0x0, 0x29 }}
, {8, 0x46B8, { 0, 0, 0, 0, 0x0, 0x28 }}
};

const HardwareVersion * currentVersion;

Timeout timeout;

/*
 * Initialize the application.
 */
void setup()
{
    // XXX read some ID pins to determine which version is attached
    currentVersion = & hardwareVersion[0];
    volatile char v = getAppVersion()[0];
    v++;
    bcu.begin(131, currentVersion->hardwareVersion[5], 0x28);  // we are a MDT shutter/blind actuator, version 2.8
    memcpy(userEeprom.order, currentVersion->hardwareVersion, sizeof(currentVersion->hardwareVersion));

    pinMode(PIN_INFO, OUTPUT);	// Info LED
    pinMode(PIN_RUN,  OUTPUT);	// Run LED

    // Running the controller in a closed housing makes these LEDs useless - they just consume power
    // additionally at the moment the rol-jal application does not make use of these LEDs
    // check config file to toggle the use
#ifndef USE_DEV_LEDS
	digitalWrite(PIN_INFO, 0);
	digitalWrite(PIN_RUN, 0);
#endif


    if (bcu.applicationRunning())
    {
        initApplication();
    }
    else
    {
        // no application is loaded, make sure that all relays are off
        pinMode(PIN_PWM, OUTPUT);  // configure PWM Pin as output when app is not loaded
        digitalWrite(PIN_PWM, 1);  // set PWM Pin to high so all relays will be off
    }

	timeout.start    (1);
}

/*
 * The main processing loop.
 */
void loop()
{
    int objno;
    // Handle updated communication objects
    while ((objno = nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }
    // check the periodic function of the application
    checkPeriodicFuntions();

    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}

