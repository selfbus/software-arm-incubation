/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib.h>
#include <string.h>

#include "app_in.h"
#include "params.h"
#include "channel.h"
#include "switch.h"
#include "jalo.h"
#include "input.h"

Input inputs;


#define MAX_CHANNELS 16

Channel * channelConfig[MAX_CHANNELS];


void objectUpdated(int objno)
{
    int channel = objno / 5;
    int channelObjno = objno - (channel * 5);
    if (channelObjno == 4) // change of the lock object
        channelConfig[channel]->setLock(objectRead(objno));
}

void checkPeriodic(void)
{

    for (int i = 0; i < currentVersion->noOfChannels; i++)
    {
        unsigned int value;
        bool longPressed;
        if (inputs.checkInput(i, &value, &longPressed))
        {
            Channel * channel = channelConfig[i];
            if (! channel->locked)
                channel->inputChanged(value, longPressed);
        }
    }

    for (int i = 0; i < currentVersion->noOfChannels; i++)
    {
        channelConfig[i]->checkPeriodic();
    }
}

void initApplication(void)
{
    memset (channelConfig, 0, sizeof (channelConfig));
    inputs.begin(currentVersion->noOfChannels, currentVersion->baseAddress);

    for (int i = 0; i < currentVersion->noOfChannels; i++)
    {
        byte    * configBase = & userEeprom [currentVersion->baseAddress + 4 + i * 46];
        word      channelType = * (word *) configBase;
        Channel * channel;

        switch (channelType)
        {
        case 0   : // channel is configured as switch
        case 256 : // channel is configured as switch short/long
            channel = new Switch(i, channelType, configBase); break;
        case 1 : // channel is configured as dimmer
            channel = 0; break;
        case 2 : // channel is configured as jalo
            channel = new Jalo(i); break;
        case 3 : // channel is configured as scene
            channel = 0; break;
        case 4 : // channel is configured as counter
            channel = 0; break;
        default:
            channel = 0;
        }
        channelConfig[i] = channel;
    }
}
