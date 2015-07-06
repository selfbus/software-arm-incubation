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

#include "config.h"
#include "app-rol-jal.h"
#include "channel.h"
#include "blind.h"
#include "shutter.h"
#include <sblib/eib.h>
#include <sblib/digital_pin.h>
#include <sblib/io_pin_names.h>

#ifdef HAND_ACTUATION
#include "hand_actuation.h"
#endif

//#define MEM_TEST

Channel * channels[NO_OF_CHANNELS];
#ifdef MEM_TEST
Blind b0 = Blind(0, 0);
Blind b1 = Blind(1, 1);
Blind b2 = Blind(2, 2);
Shutter b3 = Shutter(3, 3);
#endif

void objectUpdated(int objno)
{
    if (objno >= 13)
    {   // handle the com objects specific to one channel
        unsigned int channel = (objno - 13) / 20;
        Channel * chn = channels [channel];
        if (chn)
            chn->objectUpdate(objno);
    }
    else
    {   // handle global objects
        unsigned char value = objectRead(objno);
        for (unsigned int i = 0; i < NO_OF_CHANNELS; i++)
        {   Channel * chn = channels [i];
            if (chn && (objno <= 4) && chn->centralEnabled())
            {
                switch (objno)
                {
                case 0 : // move ALL channels up/down (long)
                    chn->handleMove(value);
                    break;
                case 1 : // move slat/stop for ALL channels (short)
                    chn->handleStep(value);
                    break;
                case 2 : // stop for ALL channels
                    chn->stop();
                    break;
                case 3 : // absolute position for ALL channels
                    chn->moveTo(value);
                    break;
                case 4 : // absolute slat position for ALL channels
                    if (chn->channelType() == Channel::BLIND)
                    {
                        ((Blind *) chn)->moveSlatTo(value);
                    }
                    break;
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

#ifdef HAND_ACTUATION
    int handStatus = handAct.check();
    if (handStatus != HandActuation::NO_ACTUATION)
    {
        unsigned int number = handStatus & 0xFF;
        Channel * chn = channels [number / 2];
        if ((chn != 0) && (handStatus & HandActuation::BUTTON_PRESSED))
        {
            if (number & 0x01)
            {
                if (chn->isRunning() == Channel::DOWN)
                    chn->stop();
                else
                    chn->startDown();
            }
            else
            {
                if (chn->isRunning() == Channel::UP)
                    chn->stop();
                else
                    chn->startUp();
            }
        }
    }
#endif
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
        switch (userEeprom.getUInt8(address))
        {
        case 0: channels [i] = new Blind(i, address); break;
        case 1: channels [i] = new Shutter(i, address); break;
        default :
            channels [i] = 0;
        }
    }
}
