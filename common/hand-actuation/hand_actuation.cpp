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

/*
 *  hand actuation pin configuration
 */
#ifdef HAND_ACTUATION
    const int handPins[NO_OF_HAND_PINS] =
            //  { PIN_LT5, PIN_LT6, PIN_LT1, PIN_LT2, PIN_LT3, PIN_LT4, PIN_LT7, PIN_LT8 }; // these are not correct
            { PIN_LT1, PIN_LT2, PIN_LT3, PIN_LT4, PIN_LT5, PIN_LT6, PIN_LT7, PIN_LT8 };
#endif

#define HAND_READBACK PIN_LT9
#define BLINK_TIME 500

HandActuation::HandActuation()
   : number(0)
   , mask(0x01)
   , _buttonState(0)
   , _ledState(0)
   , _blinkState(0)
{
    for (unsigned int i = 0; i < GetHandPinCount(); i++)
    {
        pinMode(handPins[i], OUTPUT);
        digitalWrite(handPins[i], false);
    }

    pinMode(HAND_READBACK, PULL_UP);

    blinkTimer.start(BLINK_TIME);
}

int HandActuation::check(void)
{
    int result = NO_ACTUATION;
    if (_handDelay.expired() || _handDelay.stopped())
    {   // check one input at a time
        bool buttonundertestingispressed = false;

        // save led state and turn off the LED for the button we are testing
        bool lastLEDState = _ledState & (1 << number);
        if (lastLEDState)
            digitalWrite(handPins[number], false);

        bool atleastonebuttonpressed = !digitalRead(HAND_READBACK); // read while the LED is off, low=>at least one button is pressed, high=>no button is pressed

        if (atleastonebuttonpressed) // at least one button is pressed, check if its the one we are testing right now (number)
        {
            // turn on all LED's, except the one for the button under testing (number)
            for (unsigned int i = 0; i < GetHandPinCount(); i++)
                if (i != number) digitalWrite(handPins[i], true);

            buttonundertestingispressed = !digitalRead(HAND_READBACK); // read while all LED's are on, except the one for our button we check right now, low=>button to check is pressed

            // restore LED states
            for (unsigned int i = 0, bitMask = 0x01; i < GetHandPinCount(); i++, bitMask = bitMask << 1)
                digitalWrite(handPins[i], _ledState & bitMask);
        }

        // restore LEDs status for the button under testing
        if (lastLEDState)
            digitalWrite(handPins[number], lastLEDState);

        if (buttonundertestingispressed)
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
        if (number == GetHandPinCount())
        {
            number = 0;
            mask  = 0x1;
        }
        _handDelay.start(DELAY_BETWEEN_BUTTONS);
    } // if (_handDelay.expired()


    if (blinkTimer.expired())
    {
        unsigned int bitMask = 0x01; // FIXME this should never have worked, bitMask is no where shifted
        blinkTimer.start(BLINK_TIME);
        for (unsigned int i = 0; i < GetHandPinCount(); i++)
        {
            if (_blinkState & bitMask)
                digitalWrite(handPins[i], blinkOnOffState);
        }
        blinkOnOffState = !blinkOnOffState;
    }
    return result;
}

int HandActuation::getButtonAndState(int& btnNumber, HandActuation::ButtonState& btnState)
{
    int result = false;
    int handStatus = this->check();

    result = (handStatus != HandActuation::NO_ACTUATION);
    if (result)
    {
        btnNumber = handStatus & 0xFF;
        btnState = HandActuation::ButtonState(handStatus & 0xFF00);
    }
    return result;
}

ALWAYS_INLINE unsigned int HandActuation::GetHandPinCount()
{
    if (sizeof(handPins) > 0)
        return (sizeof(handPins)/sizeof(handPins[0]));
    else
        return -1;
}

void HandActuation::setLedState(unsigned int led, bool state, bool blink)
{
    if (led >= GetHandPinCount())
        return;

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

void HandActuation::setallLedState(bool state)
{
    for (unsigned int i = 0; i < GetHandPinCount(); i++)
    {
        setLedState(i, state, false);
    }
}

#ifdef HAND_ACTUATION
    HandActuation handAct = HandActuation();
    Timeout HandActuation::blinkTimer;
    bool    HandActuation::blinkOnOffState = false;
#endif
