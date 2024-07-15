/*
 *  app-rol-jal.cpp - The application for the 4 channel blinds/shutter actuator
 *  acting as a MDT JAL-0410.01 (ver. 2.8)
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
#include <sblib/digital_pin.h>
#include <sblib/io_pin_names.h>
#include "hand_actuation.h"

#ifndef HAND_ACTUATION
    HandActuation* handAct = nullptr;
#else
    HandActuation* handAct = new HandActuation(&handPins[0], NO_OF_HAND_PINS, READBACK_PIN, BLINK_TIME);
#endif

Channel * channels[NO_OF_CHANNELS];
#ifdef MEM_TEST
Blind b0 = Blind(0, 0);
Blind b1 = Blind(1, 1);
Blind b2 = Blind(2, 2);
Shutter b3 = Shutter(3, 3);
#endif

uint8_t channelCount()
{
    return sizeof(channels)/sizeof(channels[0]);
}

void objectUpdated(int objno)
{
    if (objno >= 13)
    {   // handle the com objects specific to one channel
        unsigned int channel = (objno - 13) / 20;
        Channel * chn = channels [channel];
        if (chn)
            chn->objectUpdateCh(objno);
    }
    else
    {   // handle global objects
        unsigned char value = bcu.comObjects->objectRead(objno);
        for (uint8_t i = 0; i < channelCount(); i++)
        {
            Channel * chn = channels [i];
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

void checkHandActuation(void)
{
    if (handAct == nullptr)
    {
        return;
    }

    int btnNumber;
    HandActuation::ButtonState btnState;
    bool processHandActuation = (handAct->getButtonAndState(btnNumber, btnState)); // changed a button its state?
    processHandActuation &= (btnState == HandActuation::BUTTON_PRESSED); // was a button pressed?

    if (!processHandActuation)
    {
        return;
    }

    Channel * chn = channels [btnNumber / 2];
    if ((chn == nullptr) || (!chn->isHandModeAllowed())) // is a channel associated with the pressed button?
    {
        return;
    }

    if (btnNumber & 0x01)
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

void checkPeriodicFuntions(void)
{
    for (uint8_t i = 0; i < channelCount(); i++)
    {
        Channel * chn = channels[i];
        if (chn)
            chn->periodic();
    }

    if (PWMDisabled.expired() || PWMDisabled.stopped()) // if (PWMDisabled.started () && PWMDisabled.expired()) // this was never triggered
    {
        Channel::startPWM();  // re-enable the PWM
    }

    checkHandActuation();
}

void getChannelPositions(short channelPositions[], short channelSlatPositions[]){
    for (uint8_t i = 0; i < channelCount(); i++)
    {
        if (channels[i] != nullptr)
        {
            channelPositions[i] = channels[i]->currentPosition();
            if(channels[i]->channelType() == Channel::BLIND){
                channelSlatPositions[i] = ((Blind *) channels[i])->currentSlatPosition();
            }
        }
    }
}

void initApplication(short channelPositions[], short channelSlatPositions[])
{
    Channel::initPWM(PIN_PWM);  // configure digital pin PIO3_2 (PIN_PWM) and timer16_0 for PWM

    unsigned int address = currentVersion.baseAddress;

    for (uint8_t i = 0; i < sizeof(outputPins)/sizeof(outputPins[0]); i++)
    {
        pinMode(outputPins[i], OUTPUT);  // first set pinmode to OUTPUT
        digitalWrite(outputPins[i], Channel::OUTPUT_LOW);  // then set pin to low, otherwise relays for deactivated shutter/blind channels (in ETS) will switch on
    }

    for (uint8_t i = 0; i < channelCount(); i++, address += EE_CHANNEL_CFG_SIZE)
    {
        short position;
        short slatPosition;
        if (channelPositions != nullptr)
        {
            position = channelPositions[i];
        }
        else
        {
            position = 0;
        }

        if (channelSlatPositions != nullptr)
        {
            slatPosition = channelSlatPositions[i];
        }
        else
        {
            slatPosition = 0;
        }

        switch (bcu.userEeprom->getUInt8(address))
        {
        case 0: channels [i] = new Blind(i, address, position, slatPosition); break;
        case 1: channels [i] = new Shutter(i, address, position); break;
        default :
            channels [i] = 0;
        }

        if (channels[i] != nullptr)
        {
            channels[i]->setHandActuation(handAct);
        }
    }
}

void stopApplication()
{
    // ToDo: alles nötige veranlassen für den Shutdown
    if (handAct != nullptr)
    {
       // switch all hand actuation LEDs off, to save some power
       handAct->setallLedState(false);
    }

    // finally stop the bcu
    bcu.end();
}
