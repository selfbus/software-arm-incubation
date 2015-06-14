/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <hand_actuation.h>
#include <sblib/digital_pin.h>
#include <sblib/timer.h>

const int handPins[NO_OF_OUTPUTS] =
    { PIO2_1, PIO0_3, PIO2_4, PIO2_5, PIO3_5, PIO3_4, PIO1_10, PIO0_11 };

HandActuation::HandActuation()
   : number(0)
   , mask(0x01)
   , _buttonState(0)
{
}

int HandActuation::check(void)
{
    int result = -1;
    if (_handDelay.expired() || _handDelay.stopped())
    {   // check one input at a time
        unsigned int stateOne = digitalRead(PIO2_3);
        digitalWrite(handPins[number], !digitalRead(handPins[number]));
        delayMicroseconds(10);
        unsigned int stateTwo = digitalRead(PIO2_3);
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
        if (number == NO_OF_OUTPUTS)
        {
            number = 0;
            mask  = 0x1;
            _handDelay.start(DELAY_AT_END);
        }
        else
            _handDelay.start(DELAY_BETWEEN_BUTTONS);
    }
    return result;
}
