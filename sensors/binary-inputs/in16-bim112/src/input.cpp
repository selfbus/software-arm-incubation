/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/user_memory.h>
#include "input.h"

//#include "LedIndication.h"


void Input::begin(int noOfChannels, int baseAddress)
{
    this->noOfChannels = noOfChannels;
    this->debounceTime = userEeprom.getUInt16(baseAddress);
    inputState = 0;
    scan();
    int mode;
#ifdef INVERT
    	mode = INPUT | HYSTERESIS | PULL_UP;
#else
    	mode = INPUT | HYSTERESIS | PULL_DOWN;
#endif
    for (int i = 0; i < noOfChannels; i++)
    {
        unsigned int mask = 1 << i;
        pinMode(inputPins[i], mode);
        inputDebouncer[i].init(inputState & mask);
    }
    //leds.begin();
}

void Input::scan(void)
{
    for (unsigned int i = 0; i < noOfChannels; i++)
    {
#ifdef INVERT
      	if (digitalRead(inputPins[i]))
       	{
       		inputState &= 0xffff ^ (1 << i);
       	}
       	else
       	{
       		inputState |= 1 << i;
       	}

#else
       	if (digitalRead(inputPins[i]))
       	{
       		inputState |= 1 << i;
       	}
       	else
       	{
       		inputState &= 0xffff ^ (1 << i);
       	}
#endif
    }
}

bool Input::checkInput(unsigned int channel, unsigned int * value)
{
    unsigned int mask = 1 << channel;
    unsigned int lastValue = inputDebouncer[channel].value();
    *value = inputDebouncer[channel].debounce(inputState & mask, debounceTime);
    return lastValue != *value;
}
