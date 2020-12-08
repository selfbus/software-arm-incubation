/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef HAND_ACTUATION_H_
#define HAND_ACTUATION_H_

#include "config.h"
#include <sblib/timeout.h>

#ifndef NO_OF_HAND_PINS
#   define NO_OF_HAND_PINS NO_OF_CHANNELS //FIXME modules should work without external symbols/variables
#   pragma message ( "FIXME NO_OF_HAND_PINS not defined!, defaulting to NO_OF_CHANNELS" )
#endif

extern const int handPins[NO_OF_HAND_PINS];

class HandActuation
{
public:
    enum ButtonState {NO_ACTUATION = -1, BUTTON_PRESSED = 0x100, BUTTON_RELEASED = 0x200};
    HandActuation();
    int check(void);
    int getButtonAndState(int& btnNumber, HandActuation::ButtonState& btnState); // returns true in case a button is pressed or was released
    bool ledState(unsigned int led);
    bool blinkState(unsigned int led);
    void setLedState(unsigned int led, bool state, bool blinking = false);
    void setallLedState(bool state);

protected:
    static const unsigned int DELAY_AT_END          = 10;
    static const unsigned int DELAY_BETWEEN_BUTTONS = 10;
    static Timeout blinkTimer;
    static bool blinkOnOffState;
    unsigned int GetHandPinCount();

    unsigned int  number;
    unsigned char mask;
    unsigned char _buttonState;
    unsigned char _ledState;
    unsigned char _blinkState;
    Timeout _handDelay;
};

extern HandActuation handAct;

inline bool HandActuation::ledState(unsigned int led)
{
    return _ledState & (1 << led) ? true : false;
}

inline bool HandActuation::blinkState(unsigned int led)
{
    return _blinkState & (1 << led) ? true : false;
}
#endif /* HAND_ACTUATION_H_ */
