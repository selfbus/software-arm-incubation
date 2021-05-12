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
#include <sblib/io_pin_names.h>

const int handPins[NO_OF_HAND_PINS] =
	// Pinbelegung für Hardware ???
//  { PIN_LT5, PIN_LT6, PIN_LT1, PIN_LT2, PIN_LT3, PIN_LT4, PIN_LT7, PIN_LT8 };

	// LedTaster_4TE_3.54 : links oben = Kanal 1 nach unten aufsteigende Kanalnummer ; rechts oben = Kanal 5 ; nach unten aufsteigende Kanalnummer
    { PIN_LT1, PIN_LT2, PIN_LT3, PIN_LT4, PIN_LT5, PIN_LT6, PIN_LT7, PIN_LT8 };

#define HAND_READBACK PIN_LT9
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
#ifdef HAND_DEBUG
        unsigned int stateOne = 0;
        unsigned int stateTwo = (_inputState & mask) ? 1 : 0;
#else
        unsigned int stateOne = digitalRead(HAND_READBACK);
        digitalWrite(handPins[number], !digitalRead(handPins[number]));
        delayMicroseconds(10);
        unsigned int stateTwo = digitalRead(HAND_READBACK);
        digitalWrite(handPins[number], !digitalRead(handPins[number]));
#endif
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
