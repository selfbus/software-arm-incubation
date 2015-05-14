/*
 *  app_main.cpp - The application's main.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_out8.h"
#include "com_objs.h"

#include <sblib/eib.h>
#include "outputs.h"

// Digital pin for LED
#define PIO_YELLOW PIO2_6
#define PIO_GREEN  PIO3_3

// Output pins
#ifdef BI_STABLE
const int outputPins[NO_OF_OUTPUTS] =
    { PIO2_10, PIO0_7 //  1,  2
    , PIO0_5,  PIO0_2 //  3,  4
    , PIO0_6,  PIO3_2 //  5,  6
    , PIO2_9,  PIO2_2 //  7,  8

    , PIO1_5,  PIO1_0 //  9, 10
    , PIO1_6,  PIO1_7 // 11, 12
    , PIO3_1,  PIO1_1 // 13, 14
    , PIO1_2,  PIO3_0 // 15, 16
    };
#else
const int outputPins[NO_OF_OUTPUTS] =
    { PIO2_2, PIO0_7, PIO2_10, PIO2_9, PIO0_2, PIO0_8, PIO0_9, PIO2_11 };
#endif

ObjectValues& objectValues = *(ObjectValues*) (userRamData + UR_COM_OBJ_VALUE0);

//#define IO_TEST

/*
 * Initialize the application.
 */
void setup()
{
    bcu.begin(4, 0x2060, 1); // We are a "Jung 2138.10" device, version 0.1

    pinMode(PIO2_6, OUTPUT);	// Info LED
    pinMode(PIO3_3, OUTPUT);	// Run LED
    // Configure the output pins
    for (int channel = 0; channel < NO_OF_OUTPUTS; ++channel)
    {
        digitalWrite(outputPins[channel], 0);
        pinMode(outputPins[channel], OUTPUT);
    }
#ifdef IO_TEST
    for (unsigned int i = 0; i < NO_OF_OUTPUTS; i++)
    {
        digitalWrite(outputPins[i], 1);
#ifdef HAND_ACTUATION
        if (i < NO_OF_CHANNELS)
            digitalWrite(handPins[i], 1);
#endif
        delay(1000);
        digitalWrite(outputPins[i], 0);
#ifdef HAND_ACTUATION
        if (i < NO_OF_CHANNELS)
            digitalWrite(handPins[i], 0);
        delay(1000);
#endif
    }
#endif
#ifndef BI_STABLE
    pinMode(PIO1_2, OUTPUT);
    digitalWrite(PIO1_2, 1);
    pinInterruptMode(PIO0_5, INTERRUPT_EDGE_FALLING | INTERRUPT_ENABLED);
#endif
    initApplication();
#ifndef BI_STABLE
    enableInterrupt(EINT0_IRQn);
#endif
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
    // check if any of the timeouts for an output has expire and react on them
    checkTimeouts();

    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}
