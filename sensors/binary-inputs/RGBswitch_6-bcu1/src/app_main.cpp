/*
 *  Copyright (c) 2018 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_RGBswitch.h"
#include "com_objs.h"
#include "params.h"

#include <sblib/eib.h>
#include <sblib/timeout.h>
#include <sblib/eib/sblib_default_objects.h>



// Digital pin for LED
#define PIO_LED PIO3_3

#define DIRECT_IO

// Debouncers for inputs
Debouncer inputDebouncer[NUM_CHANNELS];

const byte* channelParams = userEepromData + (EE_CHANNEL_PARAMS_BASE - USER_EEPROM_START);
const byte* channelTimingParams = userEepromData + (EE_CHANNEL_TIMING_PARAMS_BASE - USER_EEPROM_START);
const byte* LEDparams = userEepromData + (EE_LED_PARAMS_BASE - USER_EEPROM_START);

#ifdef DIRECT_IO
// Input pins
const int inputPins[] =
    { PIO1_7, PIO1_10, PIO0_1, PIO2_4, PIO2_8, PIO2_10 };

void setupIO(void)
{
    // Configure the input pins and initialize the debouncers with the current
    // value of the pin.
    for (int channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        pinMode(inputPins[channel], INPUT | HYSTERESIS | PULL_UP);
        inputDebouncer[channel].init(digitalRead(inputPins[channel]));
    }
}

void scanIO(void)
{

}

int readIO(int channel)
{
    return !digitalRead(inputPins[channel]);
}

#else
#include <sblib/spi.h>

SPI spi(SPI_PORT_0);
static unsigned int ioState;

void setupIO(void)
{
   ioState = 0;
   pinMode(PIO0_8,  OUTPUT | SPI_MISO);  // IO6
   pinMode(PIO0_9,  OUTPUT);             // IO7
   pinMode(PIO2_11, OUTPUT | SPI_CLOCK); // IO8

   spi.setClockDivider(128);
   spi.begin();
   spi.setDataSize(SPI_DATA_8BIT);
}

void scanIO(void)
{
    digitalWrite(PIO0_9, 0);
    digitalWrite(PIO0_9, 1);
    ioState = spi.transfer(0x00);
}

int readIO(int channel)
{
    return ioState & (1 << channel);

}
#endif

/**
 * Application setup
 */
void setup()
{
    bcu.begin(76, 0x474, 2); // We are a "Selfbus RGB Taster" device, version 0.2

    // onboard LEDs ausschalten
    pinMode(PIO1_0, OUTPUT);
    digitalWrite(PIO1_0, 0);
    pinMode(PIO2_10, OUTPUT);
    digitalWrite(PIO2_10, 0);

    setupIO();
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
        scanIO();
        for (int channel = 0; channel < NUM_CHANNELS; ++channel)
        {
            inputDebouncer[channel].debounce(readIO(channel), debounceTime);
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

    scanIO();
    // Handle the input pins
    for (channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        lastValue = inputDebouncer[channel].value();
        value = inputDebouncer[channel].debounce(readIO(channel), debounceTime);

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
