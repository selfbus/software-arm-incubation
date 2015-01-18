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

#define NUM_CHANNELS 8

#include <sblib/eib.h>
#include "outputs.h"

// Digital pin for LED
#define PIO_YELLOW PIO2_6
#define PIO_GREEN  PIO3_3

// Output pins
const int outputPins[NO_OF_CHANNELS] =
    { PIO2_2, PIO0_7, PIO2_10, PIO2_9, PIO0_2, PIO0_8, PIO0_9, PIO2_11 };
#ifdef HAND_ACTUATION
const int handPins[NO_OF_CHANNELS] =
    { PIO2_1, PIO0_3, PIO2_4, PIO2_5, PIO3_5, PIO3_4, PIO1_10, PIO0_11 };
#endif

ObjectValues& objectValues = *(ObjectValues*) (userRamData + UR_COM_OBJ_VALUE0);



/*
 * Initialize the application.
 */
void setup()
{
    bcu.begin(4, 0x2060, 1); // We are a "Jung 2138.10" device, version 0.1

    pinMode(PIO2_6, OUTPUT);	// Info LED
    pinMode(PIO3_3, OUTPUT);	// Run LED
    // Configure the output pins
    for (int channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        pinMode(outputPins[channel], OUTPUT);
#ifdef HAND_ACTUATION
        pinMode(handPins[channel], OUTPUT);
#endif
    }
#ifdef HAND_ACTUATION
    pinMode(PIO2_3, INPUT | PULL_UP | HYSTERESIS);
#endif
    pinMode(PIO1_2, OUTPUT);
    digitalWrite(PIO1_2, 1);
    pinInterruptMode(PIO0_5, INTERRUPT_EDGE_FALLING | INTERRUPT_ENABLED);
    initApplication();
    enableInterrupt(EINT0_IRQn);
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
