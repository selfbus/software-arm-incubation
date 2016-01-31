/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/user_memory.h>
#include <sblib/digital_pin.h>
#include "input.h"

#include "LedIndication.h"
#include <sblib/io_pin_names.h>

Led_Indication leds(SPI_PORT_1, PIN_LT9, PIN_LT1, PIN_LT2, PIN_LT3);
// Input pins
const int inputPins[] =
{
#ifndef __LPC11UXX__
  PIO2_5, //  A0
  PIO3_5, //  A1
  PIO0_7, //  A2
  PIO2_9, //  A3

  PIO1_11, // B0
  PIO1_4, //  B1
  PIO1_2, //  B2
  PIO1_1, //  B3

  PIO1_7, //  C0
  PIO1_6, //  C1
  PIO1_5, //  C2
  PIO3_2, //  C3

  PIO2_8, //  D0
  PIO2_7, //  D1
  PIO2_6, //  D2
  PIO3_3, //  D3
#else
  PIN_PWM
, PIN_APRG
, PIN_IO1

, PIN_IO2
, PIN_IO3
, PIN_IO4
, PIO_SDA
, PIN_IO5

, PIN_IO14
, PIN_IO15
, PIN_IO13
, PIN_IO11

, PIN_IO9
, PIN_IO10
, PIN_TX
, PIN_RX
#endif
};

void Input::begin(int noOfChannels, int baseAddress)
{
    this->noOfChannels = noOfChannels;
    this->debounceTime = userEeprom.getUInt16(baseAddress);
    inputState = 0;
    scan();
    for (int i = 0; i < noOfChannels; i++)
    {
        unsigned int mask = 1 << i;
        pinMode(inputPins[i], INPUT | HYSTERESIS | PULL_UP);
        inputDebouncer[i].init(inputState & mask);
    }
    leds.begin();
}

void Input::scan(void)
{
    for (unsigned int i = 0; i < noOfChannels; i++)
    {
        if (digitalRead(inputPins[i]))
        {
            inputState |= 1 << i;
        }
        else
        {
            inputState &= 0xffff ^ (1 << i);
        }
    }
}

bool Input::checkInput(unsigned int channel, unsigned int * value)
{
    unsigned int mask = 1 << channel;
    unsigned int lastValue = inputDebouncer[channel].value();
    *value = inputDebouncer[channel].debounce(inputState & mask, debounceTime);
    return lastValue != *value;
}
