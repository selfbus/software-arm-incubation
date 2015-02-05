/*
 *  outputs.cpp - 
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib.h>
#include "config.h"
#include "outputs.h"

#ifdef HAND_ACTUATION
const int handPins[NO_OF_CHANNELS * 2] =
    { PIO2_1, PIO0_3, PIO2_4, PIO2_5, PIO3_5, PIO3_4, PIO1_10, PIO0_11 };
#endif

Outputs::Outputs(void)
{
#ifdef ZERO_DETECT
    _state           = 0; // wait for next required switch
    _port_0_set = _port_0_clr = _port_2_set = _port_2_clr = 0;
    timer32_0.begin();
    timer32_0.prescaler((SystemCoreClock / 100000) - 1);
    timer32_0.matchMode(MAT3, RESET | STOP);
    timer32_0.matchMode(MAT0, INTERRUPT);
    timer32_0.matchMode(MAT1, INTERRUPT);
    timer32_0.match(MAT0, zeroDetectSetDelay);
    timer32_0.match(MAT1, zeroDetectClrDelay);
    timer32_0.match(MAT3, ZD_RESET);
    timer32_0.interrupts();
#endif

#ifdef HAND_ACTUATION
    _handCount = 0;
    for (int channel = 0; channel < NO_OF_CHANNELS * 2; ++channel)
    {
        pinMode(handPins[channel], OUTPUT);
    }
    pinMode(PIO2_3, INPUT | PULL_UP | HYSTERESIS);
    pinMode(PIO1_2, OUTPUT);
    digitalWrite(PIO1_2, 1);
#endif
}

void Outputs::init(void)
{
#ifdef ZERO_DETECT
    enableInterrupt(EINT0_IRQn);
    pinInterruptMode(PIO0_5, INTERRUPT_EDGE_FALLING | INTERRUPT_ENABLED);
#endif
}
