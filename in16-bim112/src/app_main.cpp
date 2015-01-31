/*
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_in.h"

#include "com_objs.h"
#include "params.h"

#include <sblib/eib.h>
#include <sblib/eib/user_memory.h>
#include <string.h> /* for memcpy() */


// Digital pin for LED
#define PIO_LED PIO0_7

// Input pins
static const int inputPins[] = { PIO2_3, PIO2_2, PIO2_1, PIO2_0 };

// Debouncers for inputs
Debouncer inputDebouncer[NUM_CHANNELS];

ObjectValues& objectValues = *(ObjectValues*) (userRamData + UR_COM_OBJ_VALUE0);
const byte* channelParams = userEepromData + (EE_CHANNEL_PARAMS_BASE - USER_EEPROM_START);
const byte* channelTimingParams = userEepromData + (EE_CHANNEL_TIMING_PARAMS_BASE - USER_EEPROM_START);

// Hardware version. Must match the product_serial_number in the VD's table hw_product
const byte hardwareVersion[] = { 0, 0, 0, 0, 0, 30 };

/**
 * Application setup
 */
void setup()
{
    bcu.begin(131, 0x0030, 0x20);  // we are a MDT binary input, version 2.0

    memcpy(userEeprom.order, hardwareVersion, 6);

    pinMode(PIO_LED, OUTPUT);
    digitalWrite(PIO_LED, 0);

    // Configure the input pins and initialize the debouncers with the current
    // value of the pin.
    for (int channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        pinMode(inputPins[channel], INPUT | HYSTERESIS | PULL_UP);
        inputDebouncer[channel].init(digitalRead(inputPins[channel]));
    }
}

/**
 * The application's main.
 */
void loop()
{


    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}
