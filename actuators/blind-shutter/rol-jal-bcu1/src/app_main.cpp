/*
 *  app_main.cpp - The application's main.
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#include <sblib/eibBCU1.h>
#include <sblib/io_pin_names.h>
#include "app-rol-jal.h"
#include "config.h"

BCU1 bcu = BCU1();

/**
 * Initialize the application.
 */
BcuBase* setup()
{
    bcu.begin(4, 0x2060, 1); // We are a "Jung 2138.10" device, version 0.1
#ifdef DEBUG
    pinMode(PIN_INFO, OUTPUT);	// Info LED
    pinMode(PIN_RUN,  OUTPUT);	// Run LED
#endif

    initApplication();
    return (&bcu);
}

/**
 * The main processing loop.
 */
void loop()
{
    int objno;
    // Handle updated communication objects
    while ((objno = bcu.comObjects->nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }
    // check the periodic function of the application
    checkPeriodicFuntions();

    // Sleep up to 1 millisecond if there is nothing to do
    if (bcu.bus->idle())
        waitForInterrupt();
}
