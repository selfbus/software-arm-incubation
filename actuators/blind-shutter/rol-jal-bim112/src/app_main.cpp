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
#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>
#include "app-rol-jal.h"
#include <string.h>

#ifdef HAND_ACTUATION
#include "hand_actuation.h"
#endif

// Hardware version. Must match the product_serial_number in the VD's table hw_product
const HardwareVersion hardwareVersion[] =
{ {4, 0x4578, { 0, 0, 0, 0, 0x0, 0x29 }}
, {8, 0x46B8, { 0, 0, 0, 0, 0x0, 0x28 }}
};

const HardwareVersion * currentVersion;

/*
 * Initialize the application.
 */

Timeout timeout;

void setup()
{
    // XXX read some ID pins to determine which version is attached
    currentVersion = & hardwareVersion[0];
    bcu.begin(131, currentVersion->hardwareVersion[5], 0x28);  // we are a MDT shutter/blind actuator, version 2.8
    memcpy(userEeprom.order, currentVersion->hardwareVersion, sizeof(currentVersion->hardwareVersion));

    pinMode(PIN_INFO, OUTPUT);	// Info LED
    pinMode(PIN_RUN,  OUTPUT);	// Run LED
    initApplication();
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
    if (timeout.started() && timeout.expired())
    {
    	timeout.start(1000);
    	digitalWrite(PIN_INFO, !digitalRead(PIN_INFO));
    }
}

void loop_test(void)
{
    static int i = -1;
    if (i == -1)
    {
        pinMode(PIN_PWM, OUTPUT);
        digitalWrite(PIN_PWM, 0);
        for (i = 0; i < NO_OF_OUTPUTS; i++)
        {
            pinMode(outputPins[i], OUTPUT);
            digitalWrite(outputPins[i], 0);
#ifdef HAND_ACTUATION
            pinMode(handPins[i], OUTPUT);
            digitalWrite(handPins[i], 0);
#endif // HAND
        }
        i = 0;
    }
    else if (i < (2 * NO_OF_OUTPUTS))
    {
        if (timeout.expired ())
        {
            unsigned int n = i >> 1;
            timeout.start  (500);
            digitalWrite (outputPins[n], !(i & 0x01));
#ifdef HAND_ACTUATION
            digitalWrite (handPins[n], !(i & 0x01));
#endif // HAND
            i++;
        }
    }
}
