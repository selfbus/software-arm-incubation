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

#define pinOff(ch) (ch*2+1)
#define pinOn(ch)  (ch*2)

unsigned int OutputsBiStable::updateOutput(unsigned int channel)
{
    unsigned int mask      = 1 << channel;
    unsigned int state     = (mask & (_relayState ^ _inverted)) >> channel;
    unsigned int prevState = (mask & (_prevRelayState ^  _inverted)) >> channel;
    unsigned int value     = (state ^ prevState) & state;

    if (!(state ^ prevState))
        return false; // nothing to do

    if (value)
    {
        digitalWrite(outputPins_[pinOff(channel)], 0);
        digitalWrite(outputPins_[pinOn(channel)],  1);
    }
    else
    {
        digitalWrite(outputPins_[pinOn(channel)],  0);
        digitalWrite(outputPins_[pinOff(channel)], 1);
    }

    _prevRelayState ^= mask; // toggle the bit of the channel we changed
    _pwm_timeout.start(ON_DELAY);

#ifdef HAND_ACTUATION
    if (handAct2_ != nullptr)
        handAct2_->setLedState(channel, value);
#endif
    return true;
}

void OutputsBiStable::checkPWM(void)
{
    if (_pwm_timeout.started () && _pwm_timeout.expired ())
    {
        for (unsigned int i = 0; i < outputCount(); i++)
            digitalWrite (outputPins_[i], 0);
    }
}

#ifdef BI_STABLE
    OutputsBiStable relays;
#endif


