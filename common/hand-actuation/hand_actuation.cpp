/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *                2021 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "hand_actuation.h"
#include <sblib/digital_pin.h>
#include <sblib/timer.h>
#include <sblib/io_pin_names.h>

HandActuation::HandActuation()
   : number_(0)
   , mask_(0x01)
   , buttonState_(0)
   , ledState_(0)
   , blinkState_(0)
{
}

HandActuation::HandActuation(const unsigned int* Pins, const unsigned int pinCount, const unsigned int readbackPin, const unsigned int blinkTimeMs)
    : number_(0)
    , mask_(0x01)
    , buttonState_(0)
    , ledState_(0)
    , blinkState_(0)
{
    handPins_ = (unsigned int*) Pins;
    pinCount_ = pinCount;
    for (unsigned int i = 0; i < pinCount; i++)
    {
        pinMode(Pins[i], OUTPUT);
        digitalWrite(Pins[i], false);
    }

    readbackPin_ = readbackPin;
    pinMode(readbackPin_, PULL_UP);

    blinkTimeMs_ = blinkTimeMs;
    blinkTimer.start(blinkTimeMs_);
}

int HandActuation::check(void)
{
    int result = NO_ACTUATION;
    if (handDelay_.expired() || handDelay_.stopped())
    {   // check one input at a time
        bool buttonundertestingispressed = false;

        // save led state and turn off the LED for the button we are testing
        bool lastLEDState = ledState_ & (1 << number_);
        if (lastLEDState)
            digitalWrite(handPins_[number_], false);

        bool atleastonebuttonpressed = !digitalRead(readbackPin_); // read while the LED is off, low=>at least one button is pressed, high=>no button is pressed

        if (atleastonebuttonpressed) // at least one button is pressed, check if its the one we are testing right now (number)
        {
            // turn on all LED's, except the one for the button under testing (number)
            for (unsigned int i = 0; i < GetHandPinCount(); i++)
                if (i != number_) digitalWrite(handPins_[i], true);
            delayMicroseconds(5);   // this delay is needed for compiler settings other then -O0 (Optimize Level None), otherwise detection doesn't work 100%
                                    // works also with delayMicroseconds(1);
            buttonundertestingispressed = !digitalRead(readbackPin_); // read while all LED's are on, except the one for our button we check right now, low=>button to check is pressed

            // restore LED states
            for (unsigned int i = 0, bitMask = 0x01; i < GetHandPinCount(); i++, bitMask = bitMask << 1)
            {
                if (blinkState_ & bitMask)
                    digitalWrite(handPins_[i], blinkOnOffState);
                else
                    digitalWrite(handPins_[i], ledState_ & bitMask);
            }
        }
        else
        {
            // restore LED status for the button under testing
            if ((blinkState_ & (1 << number_)))
                digitalWrite(handPins_[number_], blinkOnOffState);
            else
                digitalWrite(handPins_[number_], lastLEDState);
        }

        if (buttonundertestingispressed)
        {   // this button is currently pressed
            result = number_;
            if (! (buttonState_ & mask_))
                // this button was not pressed before
                result |= BUTTON_PRESSED;
            buttonState_ |= mask_;
        }
        else if (buttonState_ & mask_)
        {
            result = number_ | BUTTON_RELEASED;
            buttonState_ &= ~mask_;
        }
        number_++;
        mask_ <<= 1;
        if (number_ == GetHandPinCount())
        {
            number_ = 0;
            mask_  = 0x1;
            handDelay_.start(delayAtEndMs); // FIXME test this
        }
        else
            handDelay_.start(delayBetweenButtonsMs);
    } // if (_handDelay.expired()


    if (blinkTimer.expired() || blinkTimer.stopped())
    {
        for (unsigned int i = 0, bitMask = 0x01; i < GetHandPinCount(); i++, bitMask = bitMask << 1)
        {
            if (blinkState_ & bitMask)
                digitalWrite(handPins_[i], blinkOnOffState);
        }
        blinkOnOffState = !blinkOnOffState;
        blinkTimer.start(blinkTimeMs_);
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
   return pinCount_;
}

void HandActuation::setLedState(unsigned int led, bool state, bool blink)
{
    if (led >= GetHandPinCount())
        return;

    unsigned int mask = 1 << led;
    digitalWrite(handPins_[led], state);
    if (state)
    {
        ledState_ |= mask;
        if (blink)
            blinkState_ |= mask;
    }
    else
    {
        ledState_   &= ~mask;
        blinkState_ &= ~mask;
    }
}

void HandActuation::setallLedState(bool state)
{
    for (unsigned int i = 0; i < GetHandPinCount(); i++)
    {
        setLedState(i, state, false);
    }
}


Timeout HandActuation::blinkTimer;
bool    HandActuation::blinkOnOffState = false;
