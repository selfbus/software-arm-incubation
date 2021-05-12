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

#include <sblib/io_pin_names.h>
#include <sblib/digital_pin.h>
#include "app-rol-jal.h"
#include "config.h"
#include "channel.h"
#include "blind.h"
#include "shutter.h"

#ifdef HAND_ACTUATION
#   include "hand_actuation.h"
#endif

Channel * channels[NO_OF_CHANNELS];

#ifdef HAND_ACTUATION
    HandActuation handAct = HandActuation(&handPins[0], NO_OF_HAND_PINS, READBACK_PIN, BLINK_TIME);
#endif

void objectUpdated(int objno)
{
    //FIXME implement functionality
}

void checkPeriodicFuntions(void)
{
#ifdef HAND_ACTUATION
    int btnNumber;
    HandActuation::ButtonState btnState;
    if (handAct.getButtonAndState(btnNumber, btnState))
    {
        Channel* chn = channels [btnNumber / 2];
        if (btnState ==  HandActuation::BUTTON_PRESSED)
        {
            handAct.setLedState(btnNumber, true);
            if (btnNumber & 0x01)
                chn->startUp();
            else
                chn->startDown();
        }
        else if (btnState ==  HandActuation::BUTTON_RELEASED)
        {
            chn->stop();
            handAct.setLedState(btnNumber, false);
        }
    }
#endif
    //FIXME implement functionality

}

void initApplication(void)
{
    //FIXME implement functionality

    pinMode(PIN_PWM, OUTPUT); //FIXME needs PWM implementation // pinMode(PIN_PWM, OUTPUT_MATCH);
    digitalWrite(PIN_PWM, 0);

    channels [0] = new Blind(0);
    channels [1] = new Blind(1);
    channels [2] = new Shutter(2);
    channels [3] = new Shutter(3);
}
