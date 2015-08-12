/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *                2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib.h>
#include <sblib/timeout.h>
#include <string.h>

#include "app_in.h"
#include "channel.h"
#include "switch.h"
#include "jalo.h"
#include "dimmer.h"
#include "scene.h"
#include "counter.h"
#include "input.h"
#include "logic.h"

Input inputs;

Channel * channelConfig[MAX_CHANNELS];
Logic * logicConfig[MAX_LOGIC];

void objectUpdated(int objno)
{
    int channel = objno / 5;
    int channelObjno = objno - (channel * 5);
    if (channelObjno == 4)
    { // change of the lock object
        channelConfig[channel]->setLock(objectRead(objno));
    }
    if (objno >= 80)
    {
        for (unsigned int i = 0; i < MAX_LOGIC; i++)
        {
            if (logicConfig[i])
            {
                logicConfig[i]->objectUpdated(objno);
            }
        }
    }
}

void checkPeriodic(void)
{
    inputs.scan();
    for (unsigned int i = 0; i < currentVersion->noOfChannels; i++)
    {
        unsigned int value;
        if (inputs.checkInput(i, &value))
        {
            Channel * channel = channelConfig[i];
            if (channel && !channel->isLocked())
                channel->inputChanged(value);
            for (unsigned int n = 0; n < MAX_LOGIC; n++)
            {
                if (logicConfig[n])
                {
                    logicConfig[n]->inputChanged(i, value);
                }
            }
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
    unsigned int longKeyTime = userEeprom.getUInt16(
            currentVersion->baseAddress + 2);
    unsigned int addressStartupDelay = currentVersion->baseAddress + 4 // debounce, longTime
            + channels * 46 + channels + (11 + channels) * 4 // logic config
            + 10;
    unsigned int busReturn = userEeprom[addressStartupDelay - 1] & 0x2; // bit offset is 6: means 2^(7-bit offset)
    memset(channelConfig, 0, sizeof(channelConfig));
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
                inputs.checkInput(i, &value);
            }
            waitForInterrupt();
        }
    }

    unsigned int busReturnLogic = userEeprom[addressStartupDelay - 1] & 0x01;

    for (unsigned int i = 0; i < MAX_LOGIC; i++)
    {
        if (userEeprom[currentVersion->logicBaseAddress + i * (11 + channels)]
                != 0xff)
        {
            logicConfig[i] = new Logic(currentVersion->logicBaseAddress, i,
                    channels, busReturnLogic);
        }
    }

    for (unsigned int i = 0; i < channels; i++)
    {
        unsigned int value;
        int configBase = currentVersion->baseAddress + 4 + i * 46;
        word channelType = userEeprom.getUInt16(configBase);
        Channel * channel;
        inputs.checkInput(i, &value);
        for (unsigned int n = 0; n < MAX_LOGIC; n++)
        {
            if (logicConfig[n])
            {
                logicConfig[n]->inputChanged(n, value);
            }
        }

        switch (channelType)
        {
        case 0: // channel is configured as switch
            channel = new Switch(i, longKeyTime, configBase, busReturn, value);
            break;
        case 256: // channel is configured as switch short/long
            channel = new Switch2Level(i, longKeyTime, configBase, busReturn,
                    value);
            break;
        case 1: // channel is configured as dimmer
            channel = new Dimmer(i, longKeyTime, configBase, busReturn, value);
            break;
        case 2: // channel is configured as jalo
            channel = new Jalo(i, longKeyTime, configBase, busReturn, value);
            break;
        case 3: // channel is configured as scene
            channel = new Scene(i, longKeyTime, configBase, busReturn, value);
            break;
        case 4: // channel is configured as counter
            channel = new Counter(i, longKeyTime, configBase, busReturn, value);
            break;
        default:
            channel = 0;
        }
        channelConfig[i] = channel;
    }
}
