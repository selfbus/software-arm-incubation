/*
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_in.h"
#include <sblib/eib.h>
#include <sblib/eib/user_memory.h>
#include <sblib/serial.h>
#include <string.h> /* for memcpy() */

// Hardware version. Must match the product_serial_number in the VD's table hw_product
const HardwareVersion hardwareVersion[3] =
{ {16, 0x4574, { 0, 0, 0, 0, 0x00, 0x1E }}
, { 8, 0x44D4, { 0, 0, 0, 0, 0x01, 0x1E }}
, { 4, 0x4484, { 0, 0, 0, 0, 0x01, 0x1F }}
};

const HardwareVersion * currentVersion;
/**
 * Application setup
 */
void setup()
{
#ifdef SERIAL_DEBUG
    serial.setRxPin(PIO3_1);
    serial.setTxPin(PIO3_0);
    serial.begin(115200);
    serial.println("Online\n");
#endif

    bcu.setProgPin(PIO2_11);
    bcu.setProgPinInverted(false);
    bcu.setRxPin(PIO1_8);
    bcu.setTxPin(PIO1_9);

    bcu.begin(131, 0x0030, 0x20);  // we are a MDT binary input, version 2.0

    // setup debug ports
    pinMode(PIO1_0, OUTPUT);
    pinMode(PIO2_10, OUTPUT);
    digitalWrite(PIO1_0, 0);
    digitalWrite(PIO2_10, 0);

    // XXX read some ID pins to determine which version is attached
    currentVersion = & hardwareVersion[0];
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
