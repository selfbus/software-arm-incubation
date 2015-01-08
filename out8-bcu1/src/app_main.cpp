/*
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_out8.h"
#include "com_objs.h"
//#include "params.h"

#define NUM_CHANNELS 8

#include <sblib/eib.h>

// Digital pin for LED
#define PIO_YELLOW PIO2_6
#define PIO_GREEN  PIO3_3

// Output pins
const int outputPins[NO_OF_CHANNELS] =
    { PIO2_2, PIO0_7, PIO2_10, PIO2_9, PIO0_2, PIO0_8, PIO0_9, PIO2_11 };

ObjectValues& objectValues = *(ObjectValues*) (userRamData + UR_COM_OBJ_VALUE0);

/**
 * Application setup
 */
void setup()
{
    bcu.begin(4, 0x2060, 1); // We are a "Jung 2138.10" device, version 0.1

    pinMode(PIO_YELLOW, OUTPUT);
    pinMode(PIO_GREEN,  OUTPUT);
    digitalWrite(PIO_YELLOW, 0);
    digitalWrite(PIO_GREEN,  1);

    // Configure the output pins
    for (int channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        pinMode(outputPins[channel], OUTPUT);
    }
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
    // check if any of the timeouts for an output has expire and react on them
    checkTimeouts();

    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}
