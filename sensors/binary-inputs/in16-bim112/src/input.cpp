/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "input.h"

//#include "LedIndication.h"


void Input::begin(int noOfChannels, int baseAddress)
{
    this->noOfChannels = noOfChannels;
    this->debounceTime = bcu.userEeprom->getUInt16(baseAddress);
    inputState = 0;
    scan();
    int mode;
    unsigned int mask = 0;

    mode = INPUT | HYSTERESIS;
#ifdef INVERT
    mode |= PULL_UP;
#else
    mode |= PULL_DOWN;
#endif

    for (int i = 0; i < noOfChannels; i++)
    {
#ifdef INVERT
        mask  = 1 << i;
#endif
        pinMode(inputPins[i], mode);
        inputDebouncer[i].init(inputState & mask);
    }
    //leds.begin();
}

void Input::scan(void)
{
    bool pinState;
    for (uint8_t i = 0; i < noOfChannels; i++)
    {
        pinState = digitalRead(inputPins[i]);
#ifdef INVERT
        pinState = !pinState;
#endif
        if (pinState)
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
