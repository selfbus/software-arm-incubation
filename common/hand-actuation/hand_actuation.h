/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *                2021 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef HAND_ACTUATION_H_
#define HAND_ACTUATION_H_

#include <sblib/timeout.h>

class HandActuation
{
public:
    enum ButtonState {NO_ACTUATION = -1, BUTTON_PRESSED = 0x100, BUTTON_RELEASED = 0x200};
    HandActuation(const unsigned int* Pins, const unsigned int pinCount, const unsigned int readbackPin, const unsigned int blinkTimeMs);
    int check(void);
    int getButtonAndState(int& btnNumber, HandActuation::ButtonState& btnState); // returns true in case a button is pressed or was released
    bool ledState(unsigned int led);
    bool blinkState(unsigned int led);
    void setLedState(unsigned int led, bool state, bool blinking = false);
    void setallLedState(bool state);

protected:
    static const unsigned int delayBetweenButtonsMs = 10;
    static const unsigned int delayAtEndMs = 100;
    static Timeout blinkTimer;
    static bool blinkOnOffState;
    unsigned int GetHandPinCount();
    unsigned int* handPins_;
    unsigned int pinCount_;
    unsigned int readbackPin_;
    unsigned int blinkTimeMs_;

    unsigned int  number_;
    unsigned char mask_;
    unsigned char buttonState_;
    unsigned char ledState_;
    unsigned char blinkState_;
    Timeout handDelay_;
private:
    HandActuation();
};

inline bool HandActuation::ledState(unsigned int led)
{
    return ledState_ & (1 << led) ? true : false;
}

inline bool HandActuation::blinkState(unsigned int led)
{
    return blinkState_ & (1 << led) ? true : false;
}
#endif /* HAND_ACTUATION_H_ */
