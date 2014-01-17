/*
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_in8.h"
#include "com_objs.h"
#include "params.h"

#include <sblib/eib.h>

// Digital pin for LED
#define PIO_LED PIO0_7

// Input pins
static const int inputPins[] =
    { PIO2_0, PIO2_1, PIO2_2, PIO2_3, PIO2_4, PIO2_5, PIO2_6, PIO2_7 };

// Debouncers for inputs
Debouncer inputDebouncer[NUM_CHANNELS];

ObjectValues& objectValues = *(ObjectValues*) (userRamData + UR_COM_OBJ_VALUE0);
const byte* channelParams = userEepromData + (0x01d5 - EE_CHANNEL_PARAMS_BASE);
const byte* channelTimingParams = userEepromData + (0x01f6 - EE_CHANNEL_TIMING_PARAMS_BASE);


/**
 * Application setup
 */
void setup()
{
    bcu.appData(0, 4, 0x7054, 2); // We are a "Jung 2118" device, version 0.2

    pinMode(PIO_LED, OUTPUT);
    digitalWrite(PIO_LED, 0);

    // Configure the input pins and initialize the debouncers
    for (int channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        pinMode(inputPins[channel], INPUT | HYSTERESIS | PULL_UP);
        inputDebouncer[channel].init(-1);
    }
}

/**
 * The application's main.
 */
void loop()
{
    int debounceTime = userEeprom[EE_INPUT_DEBOUNCE_TIME] >> 1;
    int objno, channel, value, lastValue;

    // Handle the input pins
    for (channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        lastValue = inputDebouncer[channel].lastValue();
        value = inputDebouncer[channel].debounce(digitalRead(inputPins[channel]), debounceTime);

        if (lastValue != value)
            inputChanged(channel, value);
    }

    // Handle updated communication objects
    while ((objno = nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }

    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}
