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

HandActuation::HandActuation(const unsigned int* Pins, const unsigned int pinCount, const unsigned int readbackPin, const unsigned int blinkTimeMs)
    : pinCount_(pinCount)
    , readbackPin_(readbackPin)
    , blinkTimeMs_(blinkTimeMs)
    , number_(0)
    , mask_(0x01)
    , buttonState_(0)
    , ledState_(0)
    , blinkState_(0)
    , delayBetweenButtonsMs_(10)
    , delayAtEndMs_(10)
{
    handPins_ = (unsigned int*) Pins;
    for (unsigned int i = 0; i < getHandPinCount(); i++)
    {
        pinMode(handPins_[i], OUTPUT);
        digitalWrite(handPins_[i], false);
    }

    pinMode(readbackPin_, PULL_UP);

    blinkTimer.start(blinkTimeMs_);
}

int HandActuation::check(void)
{
    int result = NO_ACTUATION;
    if (handDelay_.expired() || handDelay_.stopped())
    {   // check one input at a time
        bool buttonUnderTestingIsPressed = false;

        // save led state and turn off the LED for the button we are testing
        bool lastLEDState = ledState_ & (1 << number_);
        if (lastLEDState)
            digitalWrite(handPins_[number_], false);

        bool atLeastOneButtonPressed = !digitalRead(readbackPin_); // read while the LED is off, low=>at least one button is pressed, high=>no button is pressed

        if (atLeastOneButtonPressed) // at least one button is pressed, check if its the one we are testing right now (number)
        {
            // turn on all LED's, except the one for the button under testing (number)
            for (unsigned int i = 0; i < getHandPinCount(); i++)
            {
                if (i != number_)
                {
                    digitalWrite(handPins_[i], true);
                }
            }
            delayMicroseconds(5);   // this delay is needed for compiler settings other then -O0 (Optimize Level None), otherwise detection doesn't work 100%
                                    // works also with delayMicroseconds(1);
            buttonUnderTestingIsPressed = !digitalRead(readbackPin_); // read while all LED's are on, except the one for our button we check right now, low=>button to check is pressed

            // restore LED states
            for (unsigned int i = 0, bitMask = 0x01; i < getHandPinCount(); i++, bitMask = bitMask << 1)
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

        if (buttonUnderTestingIsPressed)
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
        if (number_ == getHandPinCount())
        {
            number_ = 0;
            mask_  = 0x1;
            handDelay_.start(delayAtEndMs_);
        }
        else
            handDelay_.start(delayBetweenButtonsMs_);
    } // if (_handDelay.expired()


    if (blinkTimer.expired() || blinkTimer.stopped())
    {
        for (unsigned int i = 0, bitMask = 0x01; i < getHandPinCount(); i++, bitMask = bitMask << 1)
        {
            if (blinkState_ & bitMask)
                digitalWrite(handPins_[i], blinkOnOffState);
        }
        blinkOnOffState = !blinkOnOffState;
        blinkTimer.start(blinkTimeMs_);
    }
    return result;
}

bool HandActuation::getButtonAndState(int& btnNumber, HandActuation::ButtonState& btnState)
{
    bool result = false;
    int handStatus = this->check();

    result = (handStatus != HandActuation::NO_ACTUATION);
    if (result)
    {
        btnNumber = handStatus & 0xFF;
        btnState = HandActuation::ButtonState(handStatus & 0xFF00);
    }
    return result;
}

ALWAYS_INLINE unsigned int HandActuation::getHandPinCount()
{
   return pinCount_;
}

void HandActuation::setLedState(unsigned int led, bool state, bool blink)
{
    if (led >= getHandPinCount())
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
    for (unsigned int i = 0; i < getHandPinCount(); i++)
    {
        setLedState(i, state, false);
    }
}

unsigned int HandActuation::getDelayBetweenButtonsMs()
{
    return delayBetweenButtonsMs_;
}

unsigned int HandActuation::getDelayAtEndMs()
{
    return delayAtEndMs_;
}

void HandActuation::setDelayBetweenButtonsMs(unsigned int newDelayBetweenButtonsMs)
{
    delayBetweenButtonsMs_ = newDelayBetweenButtonsMs;
}

void HandActuation::setDelayAtEndMs(unsigned int newDelayAtEndMs)
{
    delayAtEndMs_ = newDelayAtEndMs;
}

bool HandActuation::ledState(unsigned int led)
{
    return ledState_ & (1 << led) ? true : false;
}

bool HandActuation::blinkState(unsigned int led)
{
    return blinkState_ & (1 << led) ? true : false;
}

void HandActuation::testIO(const unsigned int* testPins, const unsigned int pinCount, const unsigned int blinkTimeMs)
{
    if (blinkTimer.expired() || blinkTimer.stopped())
    {
        for (unsigned int j = 0; j < pinCount; j++)
        {
            if (j == 0)
            {
                // toggle first pin high/low or low/high
                digitalWrite(testPins[j], !digitalRead(testPins[j]));
            }
            else
            {
                // all other pins "follow" first pin state
                digitalWrite(testPins[j], digitalRead(testPins[0]));
            }
        }
        blinkTimer.start(blinkTimeMs);
    }
}


Timeout HandActuation::blinkTimer;
bool    HandActuation::blinkOnOffState = false;
