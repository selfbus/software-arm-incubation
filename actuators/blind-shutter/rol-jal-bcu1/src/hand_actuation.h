/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef HAND_ACTUATION_H_
#define HAND_ACTUATION_H_

#include <channel.h>
#include <sblib/timeout.h>

extern const int handPins[NO_OF_OUTPUTS];

class HandActuation
{
public:
    enum {NO_ACTUATION = -1, BUTTON_PRESSED = 0x100, BUTTON_RELEASED = 0x200};
    HandActuation();
    int check(void);

protected:
    static const unsigned int DELAY_AT_END          = 100;
    static const unsigned int DELAY_BETWEEN_BUTTONS = 30;

    unsigned int number;
    unsigned int mask;
    unsigned int _buttonState;
    Timeout _handDelay;
};

#endif /* HAND_ACTUATION_H_ */
