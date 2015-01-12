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
#include <sblib/timeout.h>

// Digital pin for LED
#define PIO_LED PIO3_3

// Input pins
const int inputPins[] =
    { PIO2_2, PIO0_7, PIO2_10, PIO2_9, PIO0_2, PIO0_8, PIO0_9, PIO2_11 };

// Debouncers for inputs
Debouncer inputDebouncer[NUM_CHANNELS];

ObjectValues& objectValues = *(ObjectValues*) (userRamData + UR_COM_OBJ_VALUE0);
const byte* channelParams = userEepromData + (EE_CHANNEL_PARAMS_BASE - USER_EEPROM_START);
const byte* channelTimingParams = userEepromData + (EE_CHANNEL_TIMING_PARAMS_BASE - USER_EEPROM_START);


/**
 * Application setup
 */
void setup()
{
    bcu.begin(4, 0x7054, 2); // We are a "Jung 2118" device, version 0.2

    pinMode(PIO_LED, OUTPUT);
    digitalWrite(PIO_LED, 1);

    // Configure the input pins and initialize the debouncers with the current
    // value of the pin.
    for (int channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        pinMode(inputPins[channel], INPUT | HYSTERESIS | PULL_UP);
        inputDebouncer[channel].init(digitalRead(inputPins[channel]));
    }

    // Handle configured power-up delay
    unsigned int startupTimeout = calculateTime
            ( userEeprom[EE_BUS_RETURN_DELAY_BASE] >> 4
            , userEeprom[EE_BUS_RETURN_DELAY_FACT] &  0x7F
            );
    Timeout delay;
    int debounceTime = userEeprom[EE_INPUT_DEBOUNCE_TIME] >> 1;
    delay.start(startupTimeout);
    while (delay.started() && !delay.expired())
    {   // while we wait for the power on delay to expire we debounce the input channels
        for (int channel = 0; channel < NUM_CHANNELS; ++channel)
        {
            inputDebouncer[channel].debounce(digitalRead(inputPins[channel]), debounceTime);
        }
        waitForInterrupt();
    }

    initApplication();
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
        lastValue = inputDebouncer[channel].value();
        value = inputDebouncer[channel].debounce(digitalRead(inputPins[channel]), debounceTime);

        if (lastValue != value)
            inputChanged(channel, value);
    }

    // Handle updated communication objects
    while ((objno = nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }

    // Handle timed functions (e.g. periodic update)
    handlePeriodic();

    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}
