/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef ROL_JAL_BIM112_SRC_HAND_ACTUATION_H_
#define ROL_JAL_BIM112_SRC_HAND_ACTUATION_H_

#include "config.h"
#include <sblib/timeout.h>

#define NO_OF_HAND_PINS 8

extern const int handPins[NO_OF_HAND_PINS];

class HandActuation
{
public:
    enum {NO_ACTUATION = -1, BUTTON_PRESSED = 0x100, BUTTON_RELEASED = 0x200};
    HandActuation();
    int check(void);
    bool ledState(unsigned int led);
    bool blinkState(unsigned int led);
    void setLedState(unsigned int led, bool state, bool blinking = false);

protected:
    static const unsigned int DELAY_AT_END          = 10;
    static const unsigned int DELAY_BETWEEN_BUTTONS = 10;
    static Timeout blinkTimer;
    static bool blinkOnOffState;

    unsigned int  number;
    unsigned char mask;
    unsigned char _buttonState;
    unsigned char _ledState;
    unsigned char _blinkState;
    Timeout _handDelay;
#ifdef HAND_DEBUG
    unsigned char _inputState;
#endif
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
#endif /* ROL_JAL_BIM112_SRC_HAND_ACTUATION_H_ */
