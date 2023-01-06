/*
 *  app-rol-jal.cpp - The application for the 4 channel blinds/shutter actuator
 *  acting as a Jung 2204REGH
 *
 *  Copyright (C) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <app-out8-dimmer.h>
#include "config.h"
#include "channel.h"
#include <sblib/eibMASK0701.h>
#include <sblib/digital_pin.h>
#include <sblib/io_pin_names.h>
#include "hand_actuation.h"

HandActuation* handAct = new HandActuation(&handPins[0], NO_OF_HAND_PINS, READBACK_PIN, BLINK_TIME);
Channel * channels[NO_OF_CHANNELS];

void objectUpdated(int objno)
{
    const uint8_t firstChannelObjectNumber = 10;
    const uint8_t channelObjectCount = 24;

    ///\todo fill with life, below is mostly c&p from somewhere
    if (objno >= firstChannelObjectNumber)
    {   // handle the com objects specific to one channel
        unsigned int channel = (objno - firstChannelObjectNumber) / channelObjectCount;
        Channel * chn = channels [channel];
        if (chn)
            chn->objectUpdateCh(objno);
    }
    else
    {   // handle global objects
        unsigned char value = bcu.comObjects->objectRead(objno);
        for (unsigned int i = 0; i < NO_OF_CHANNELS; i++)
        {   Channel * chn = channels [i];
            if (chn && (objno <= 4) && chn->centralEnabled())
            {
                switch (objno)
                {
                }
            }
            else if (chn && (objno >= 5) && (objno <= 12))
            {
                if (objno < 9)
                    chn->handleAutomaticFunction(objno - 5, 0, value);
                else
                    chn->handleAutomaticFunction(objno - 9, 1, value);
            }
        }
    }
}

void checkPeriodicFuntions(void)
{
    for (unsigned int i = 0; i < NO_OF_CHANNELS; i++)
    {
        Channel * chn = channels[i];
        if (chn)
            chn->periodic();
    }

    if (PWMDisabled.started () && PWMDisabled.expired())
    {
        // re-enable the PWM
        timer16_0.match(MAT2, PWM_DUTY_33);
    }
}

void initApplication(void)
{
    unsigned int address = currentVersion->baseAddress;

    pinMode(PIN_PWM, OUTPUT_MATCH);  // configure digital pin PIO3_2(PWM) to match MAT2 of timer16 #0
    //pinMode(PIO1_4, OUTPUT);
    timer16_0.begin();

    timer16_0.prescaler((SystemCoreClock / 100000) - 1);
    timer16_0.matchMode(MAT2, SET);  // set the output of PIO3_2 to 1 when the timer matches MAT1
    timer16_0.match(MAT2, PWM_PERIOD);        // match MAT1 when the timer reaches this value
    timer16_0.pwmEnable(MAT2);       // enable PWM for match channel MAT1
    for (unsigned int i = 0; i < NO_OF_OUTPUTS; i++)
    {
        digitalWrite(outputPins[i], 0);
        pinMode(outputPins[i], OUTPUT);
    }
    for (unsigned int i = 0; i < NO_OF_CHANNELS; i++, address += EE_CHANNEL_CFG_SIZE)
    {
        switch (bcu.userEeprom->getUInt8(address))
        {
//        case 0: channels [i] = new Blind(i, address); break;
//        case 1: channels [i] = new Shutter(i, address); break;
        default :
            channels [i] = 0;
        }
    }
}
