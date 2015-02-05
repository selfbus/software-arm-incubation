/*
 *  MonoOutputs.cpp - 
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "MonoOutputs.h"
#include <sblib/digital_pin.h>
#include <sblib/timer.h>

#if OUTPUTTYPE == MONO
MonoOutputs outputs;
#endif

#define PWM_TIMEOUT 50
#define PWM_PERIOD     857
#define PWM_DUTY_33    (588)


// Output pins
const int outputPins[NO_OF_CHANNELS * 2] =
    { PIO2_2, PIO0_7, PIO2_10, PIO2_9, PIO0_2, PIO0_8, PIO0_9, PIO2_11 };

MonoOutputs::MonoOutputs(void) : Outputs()
{
    // Configure the output pins
    for (int channel = 0; channel < NO_OF_CHANNELS * 2; ++channel)
    {
        pinMode(outputPins[channel], OUTPUT);
    }
    pinMode(PIO3_2, OUTPUT_MATCH);  // configure digital pin PIO3_2(PWM) to match MAT2 of timer16 #0
    timer16_0.begin();

    timer16_0.prescaler((SystemCoreClock / 100000) - 1);
    timer16_0.matchMode(MAT2, SET);  // set the output of PIO3_2 to 1 when the timer matches MAT1
    timer16_0.match(MAT2, PWM_PERIOD);        // match MAT1 when the timer reaches this value
    timer16_0.pwmEnable(MAT2);       // enable PWM for match channel MAT1

    // Reset the timer when the timer matches MAT3
    timer16_0.matchMode(MAT3, RESET);
    timer16_0.match(MAT3, PWM_PERIOD);     // match MAT3 ato create 14lHz
    timer16_0.start();
    _pwmTimeout.start(PWM_TIMEOUT); // start the timer to switch back to a PWM operation
}
