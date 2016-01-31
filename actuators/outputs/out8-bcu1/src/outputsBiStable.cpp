/*
 *  outputsBiStable.cpp - Handle the update of the port pins in accordance to the
 *                need of the application
 *
 *  Copyright (C) 2014-2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "outputsBiStable.h"
#define ON_DELAY 10

void OutputsBiStable::updateOutputs(void)
{
    unsigned int mask = 0x01;
    unsigned int i;
    unsigned int state   = _relayState ^ _inverted;
	unsigned int changes = _relayState ^ _prevRelayState;

    for (i = 0; i < NO_OF_CHANNELS; i++, mask <<= 1)
    {
    	if (changes & mask)
    	{
    	    if (state & mask)
                 digitalWrite(outputPins[i*2],     1);
            else digitalWrite(outputPins[i*2 + 1], 1);
    	    _pwm_timeout.start(ON_DELAY);
#ifdef HAND_ACTUATION
            handAct.setLedState(i, state & mask);
#endif
    	}
    }
    _prevRelayState = _relayState;
}

void OutputsBiStable::checkPWM(void)
{
    if (_pwm_timeout.started () && _pwm_timeout.expired ())
    {
        for (unsigned int i = 0; i < NO_OF_OUTPUTS; i++)
            digitalWrite (outputPins[i], 0);
    }
}

#ifdef BI_STABLE
OutputsBiStable relays;
#endif


