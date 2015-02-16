/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/user_memory.h>
#include "input.h"

void Input::begin(int noOfChannels, int baseAddress)
{
    this->noOfChannels = noOfChannels;
    this->debounceTime = userEeprom.getUIn16(baseAddress);
    this->longKeyTime = userEeprom.getUIn16(baseAddress + 2);
    inputState = 0;
    for(int i = 0; i < noOfChannels; i++)
    {
        unsigned int mask = 1 << i;
        inputDebouncer[i].init(inputState & mask);
    }
    scan();
}

void Input::scan(void)
{
}

bool Input::checkInput(unsigned int channel, unsigned int * value, bool * longPressed)
{
    unsigned int mask = 1 << channel;
    unsigned int lastValue = inputDebouncer[channel].value();
    * value = inputDebouncer[channel].debounce(inputState & mask, debounceTime);
    bool changed = lastValue != * value;
    * longPressed = false;
    if (changed)
    {
        longPress[channel].start(longKeyTime);
    }
    else if (longPress[channel].expired ())
    {
        * longPressed = true;
        changed = true;
    }
    return changed;
}
