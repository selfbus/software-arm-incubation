/*
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_in.h"
#include "debug.h"
#include <sblib/eib.h>
#include <sblib/eib/user_memory.h>
#include <sblib/eib/sblib_default_objects.h>
#include <string.h> /* for memcpy() */

static const char APP_VERSION[] __attribute__((used)) = "Binary Input 1.1.21";

// Hardware version. Must match the product_serial_number in the VD's table hw_product
const HardwareVersion hardwareVersion[3] =
{
{ 16, 0x4574, 0x4868,
{ 0, 0, 0, 0, 0x00, 0x1E }, APP_VERSION },
{ 8, 0x44D4, 0x4650,
{ 0, 0, 0, 0, 0x01, 0x1E }, APP_VERSION },
{ 4, 0x4484, 0x4544,
{ 0, 0, 0, 0, 0x01, 0x1F }, APP_VERSION } };

const HardwareVersion * currentVersion;
/**
 * Application setup
 */
void setup()
{
    bcu.setProgPin(PIO2_11);
    bcu.setProgPinInverted(false);
    bcu.setRxPin(PIO1_8);
    bcu.setTxPin(PIO1_9);

    debug_init();

    bcu.begin(131, 0x0030, 0x20);  // we are a MDT binary input, version 2.0

    // XXX read some ID pins to determine which version is attached
    currentVersion = &hardwareVersion[0];
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
