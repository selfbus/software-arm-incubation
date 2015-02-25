/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib.h>
#include <sblib/timeout.h>
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

    inputs.scan();
    for (unsigned int i = 0; i < currentVersion->noOfChannels; i++)
    {
        unsigned int value;
        bool longPressed;
        if (inputs.checkInput(i, &value, &longPressed))
        {
            Channel * channel = channelConfig[i];
            if (channel && ! channel->locked)
                channel->inputChanged(value, longPressed);
        }
    }

    for (unsigned int i = 0; i < currentVersion->noOfChannels; i++)
    {
        if (channelConfig[i])
            channelConfig[i]->checkPeriodic();
    }
}

void initApplication(void)
{
    unsigned int channels = currentVersion->noOfChannels;
    unsigned int addressStartupDelay =
          currentVersion->baseAddress
        + 4 // debounce, longTime
        + channels * 46
        + channels
        + (11 + channels) * 4 // logic config
        + 10;
    unsigned int busReturn = userEeprom[addressStartupDelay - 1] & 0x40;
    memset (channelConfig, 0, sizeof (channelConfig));
    inputs.begin(channels, currentVersion->baseAddress);

    Timeout startupDelay;
    // delay in config is in seconds
    unsigned int delay = userEeprom.getUInt16(addressStartupDelay) * 1000;
    startupDelay.start(delay);
    if (delay)
    {
        while (!startupDelay.expired())
        {
            for (int unsigned i = 0; i < currentVersion->noOfChannels; i++)
            {
                unsigned int value;
                bool longPressed;
                inputs.checkInput(i, &value, &longPressed);
            }
            waitForInterrupt();
        }
    }

    for (unsigned int i = 0; i < channels; i++)
    {
        unsigned int value;
        bool      longPressed;
        int       configBase = currentVersion->baseAddress + 4 + i * 46;
        word      channelType = userEeprom.getUInt16(configBase);
        Channel * channel;
        inputs.checkInput(i, &value, &longPressed);

        switch (channelType)
        {
        case 0   : // channel is configured as switch
            channel = new Switch(i, configBase, busReturn, value); break;
        case 256 : // channel is configured as switch short/long
            channel = new Switch2Level(i, configBase, busReturn, value); break;
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
