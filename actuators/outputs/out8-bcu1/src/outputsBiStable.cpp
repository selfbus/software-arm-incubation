/*
 *  outputsBiStable.cpp - Handle the update of the port pins in accordance to the
 *                need of the application
 *
 *  Copyright (C) 2014-2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "outputsBiStable.h"

#define ON_DELAY 10 // Hongfa HFE20-1 24-1HSD-L2(359) datasheet
                    // says "Pulse Duration ms min. 50ms"
                    // tested with:
                    //  5ms,  of 8 relays work //FIXME do the testing!
                    //  6ms,  of 8 relays work
                    //  7ms,  of 8 relays work
                    //  8ms,  of 8 relays work
                    //  9ms,  of 8 relays work
                    // 10ms,  of 8 relays work

/*
 * order code Hongfa HFE20-1 24-1HSD-L2(359)
 *    1:      5mm pin
 *   24:     24VDC
 *   1H:     1 Form A
 *    S:      Plastic sealed
 *    D:      W+AgSnO2 Contact
 *   L2:     Double coils latching
 *    -:    Positive polarity
* (359):  lamp load
 */

#define pinOff(i) (i*2+1)
#define pinOn(i)  (i*2)



void OutputsBiStable::updateOutputs(void)
{
    unsigned int mask = 0x01;
    unsigned int i;
    unsigned int state   = _relayState ^ _inverted;
    unsigned int changes = _relayState ^ _prevRelayState;

    for (i = 0; i < NO_OF_CHANNELS; i++, mask <<= 1)
    {
        if (changes & mask)
        {
            if (state & mask)
            {
                digitalWrite(outputPins[pinOff(i)], 0);
                digitalWrite(outputPins[pinOn(i)],  1);
            }
            else
            {
                digitalWrite(outputPins[pinOn(i)],  0);
                digitalWrite(outputPins[pinOff(i)], 1);
            }
            _pwm_timeout.start(ON_DELAY);
#ifdef HAND_ACTUATION
            handAct.setLedState(i, state & mask);
#endif
    	}
    }
    _prevRelayState = _relayState;
}

void OutputsBiStable::checkPWM(void)
{
    if (_pwm_timeout.started () && _pwm_timeout.expired ())
    {
        for (unsigned int i = 0; i < sizeof(outputPins)/sizeof(outputPins[0]); i++)
            digitalWrite (outputPins[i], 0);
    }
}

#ifdef BI_STABLE
    OutputsBiStable relays;
#endif


