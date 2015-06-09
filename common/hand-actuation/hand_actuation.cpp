/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "hand_actuation.h"
#include <sblib/digital_pin.h>
#include <sblib/timer.h>

const int handPins[NO_OF_HAND_PINS] =
    { PIO2_1, PIO0_3, PIO2_4, PIO2_5, PIO3_5, PIO3_4, PIO1_10, PIO0_11 };
#define HAND_READBACK PIO2_3
#define BLINK_TIME 500

HandActuation::HandActuation()
   : number(0)
   , mask(0x01)
   , _buttonState(0)
   , _ledState(0)
   , _blinkState(0)
{
    for (unsigned int i = 0; i < NO_OF_HAND_PINS; i++)
    {
        pinMode(handPins[i], OUTPUT);
        digitalWrite(handPins[i], false);
    }
    blinkTimer.start(BLINK_TIME);
}

int HandActuation::check(void)
{
    int result = NO_ACTUATION;
    if (_handDelay.expired() || _handDelay.stopped())
    {   // check one input at a time
        unsigned int stateOne = digitalRead(HAND_READBACK);
        digitalWrite(handPins[number], !digitalRead(handPins[number]));
        delayMicroseconds(10);
        unsigned int stateTwo = digitalRead(HAND_READBACK);
        digitalWrite(handPins[number], !digitalRead(handPins[number]));

        if (stateOne != stateTwo)
        {   // this button is currently pressed
            result = number;
            if (! (_buttonState & mask))
                // this button was not pressed before
                result |= BUTTON_PRESSED;
            _buttonState |= mask;
        }
        else if (_buttonState & mask)
        {
            result = number | BUTTON_RELEASED;
            _buttonState &= ~mask;
        }
        number++;
        mask <<= 1;
        if (number == NO_OF_HAND_PINS)
        {
            number = 0;
            mask  = 0x1;
        }
        _handDelay.start(DELAY_BETWEEN_BUTTONS);
    }
    if (blinkTimer.expired())
    {
        unsigned int bitMask = 0x01;
        blinkTimer.start(BLINK_TIME);
        for (unsigned int i = 0; i < NO_OF_HAND_PINS; i++)
        {
            if (_blinkState & bitMask)
                digitalWrite(handPins[i], blinkOnOffState);
        }
        blinkOnOffState = !blinkOnOffState;
    }
    return result;
}

void HandActuation::setLedState(unsigned int led, bool state, bool blink)
{
    unsigned int mask = 1 << led;
    digitalWrite(handPins[led], state);
    if (state)
    {
        _ledState |= mask;
        if (blink)
            _blinkState |= mask;
    }
    else
    {
        _ledState   &= ~mask;
        _blinkState &= ~mask;
    }
}

#ifdef HAND_ACTUATION
HandActuation handAct = HandActuation();
Timeout HandActuation::blinkTimer;
bool    HandActuation::blinkOnOffState = false;
#endif
