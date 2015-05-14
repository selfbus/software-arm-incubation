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
#define HAND_ACTUATION

#include "app-rol-jal.h"
#include "channel.h"
#include "blind.h"
#include "shutter.h"

#ifdef HAND_ACTUATION
#include "hand_actuation.h"

HandActuation handAct = HandActuation();
#endif

Channel * channels[NO_OF_CHANNELS];

void objectUpdated(int objno)
{

}

void checkPeriodicFuntions(void)
{
#ifdef HAND_ACTUATION
    int handStatus = handAct.check();
    if (handStatus != HandActuation::NO_ACTUATION)
    {
        unsigned int number = handStatus & 0xFF;
        Channel * chn = channels [number / 2];
        if (handStatus & HandActuation::BUTTON_PRESSED)
        {
            if (number & 0x01)
                chn->startUp();
            else
                chn->startDown();
        }
        if (handStatus & HandActuation::BUTTON_RELEASED)
            chn->stop();
    }
#endif
}

void initApplication(void)
{
    channels [0] = new Blind(0);
    channels [1] = new Blind(1);
    channels [2] = new Shutter(2);
    channels [3] = new Shutter(3);
}
