/*
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eibBCU2.h>
#include "app_in4.h"
#include "com_objs.h"
#include "params.h"

#ifdef DEBUG
#   include <sblib/serial.h>
#endif

APP_VERSION("TSU/4.2 ", "0", "02")

// Digital pin for LED
#define PIO_LED PIO0_7

// Input pins
static const int inputPins[] = { PIO2_3, PIO2_2, PIO2_1, PIO2_0 };

// Debouncers for inputs
Debouncer inputDebouncer[NUM_CHANNELS];

const byte* channelParams;
const byte* channelTimingParams;

/**
 * Application setup
 */
BcuBase* setup()
{
    channelParams = bcu.userMemoryPtr(EE_CHANNEL_PARAMS_BASE);
    channelTimingParams = bcu.userMemoryPtr(EE_CHANNEL_TIMING_PARAMS_BASE);

    bcu.begin(2, 0x9009, 0x01);  // we are a ABB TSU/4.2 version 0.1

    pinMode(PIO_LED, OUTPUT);
    digitalWrite(PIO_LED, 0);

    // Configure the input pins and initialize the debouncers with the current
    // value of the pin.
    for (int channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        pinMode(inputPins[channel], INPUT | HYSTERESIS | PULL_UP);
        inputDebouncer[channel].init(digitalRead(inputPins[channel]));
    }
#ifdef DEBUG
    if (!serial.enabled())
    {
        serial.begin(115200);
    }
    serial.println("Selfbus TSU/4.2 in4-bcu2");
#endif

    initApplication();
    return (&bcu);
}

/**
 * The application's main.
 */
void loop()
{
    int objno;
    int value;
    int lastValue;
    int debounceTime = bcu.userEeprom->getUInt8(EE_INPUT_DEBOUNCE_TIME) >> 1;

    // Handle the input pins
    for (int channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        lastValue = inputDebouncer[channel].value();
        value = inputDebouncer[channel].debounce(digitalRead(inputPins[channel]), debounceTime);

        if (lastValue != value)
            inputChanged(channel, value);
    }

    // Handle updated communication objects
    while ((objno = bcu.comObjects->nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }

    // Sleep up to 1 millisecond if there is nothing to do
    waitForInterrupt();
}

/**
 * The processing loop while no KNX-application is loaded
 */
void loop_noapp()
{

}
