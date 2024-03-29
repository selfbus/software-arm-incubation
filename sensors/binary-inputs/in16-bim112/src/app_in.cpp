/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *                2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

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
#include "ledoutput.h"
//#include "LedIndication.h"

//extern Led_Indication leds;

MASK0701 bcu = MASK0701();

Input inputs;

Channel * channelConfig[MAX_CHANNELS];
Logic * logicConfig[MAX_LOGIC];

void objectUpdated(int objno)
{
    int channels = currentVersion->noOfChannels;
	//Lock objects
	if (objno < channels*5)
	{
		int channel = objno / 5;
		int channelObjno = objno - (channel * 5);
		if ((channelObjno == 4) && (channelConfig[channel] != nullptr))
		{ // change of the lock object
		    channelConfig[channel]->setLock(bcu.comObjects->objectRead(objno));
		}
	}

    // Logic input objects
    int logicObjStart = channels/2 * 10;
    for (int i = 0; i < MAX_LOGIC; i++)
    {
        if (objno == logicObjStart + i*3 || objno == logicObjStart + 1 + i*3 )
        {
        	if (logicConfig[i])
        	{
        		logicConfig[i]->objectUpdated(objno);
        	}
        }
    }

    //LED output objects
    if (currentVersion->ledOutputFunction)
    {
    	int ledObjStart = channels/2*10+12;
    	if (objno >= ledObjStart && objno < ledObjStart+channels)
    	{
    		int i = objno - ledObjStart;
    		Channel * channel = channelConfig[i];
    		channel->objectChanged(bcu.comObjects->objectRead(objno));
    		//LED Output

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
            {
                //leds.setStatus(i, value);
                channel->inputChanged(value);
             }
            for (unsigned int n = 0; n < MAX_LOGIC; n++)
            {
                if (logicConfig[n])
                {
                    logicConfig[n]->inputChanged(i, value);
                }
            }
        }
    }
    //leds.updateLeds();
    for (unsigned int i = 0; i < currentVersion->noOfChannels; i++)
    {
        if (channelConfig[i])
            channelConfig[i]->checkPeriodic();
    }
}

void initApplication(void)
{
    unsigned int channels = currentVersion->noOfChannels;
    unsigned int longKeyTime = bcu.userEeprom->getUInt16(
            currentVersion->baseAddress + 2);
    unsigned int addressStartupDelay = currentVersion->baseAddress + 4 // debounce, longTime
            + channels * 46 + channels + (11 + channels) * 4 // logic config
            + 10;
    unsigned int busReturn = bcu.userEeprom->getUInt8(addressStartupDelay - 1) & 0x2; // bit offset is 6: means 2^(7-bit offset)
    memset(channelConfig, 0, sizeof(channelConfig));
    inputs.begin(channels, currentVersion->baseAddress);

    Timeout startupDelay;
    // delay in config is in seconds
    unsigned int delay = bcu.userEeprom->getUInt16(addressStartupDelay) * 1000;
    if (delay > 60000)
    {
        delay = 60000;
    }
    startupDelay.start(delay);
    if (delay)
    {
        while (!startupDelay.expired())
        {
            inputs.scan();
            for (int unsigned i = 0; i < currentVersion->noOfChannels; i++)
            {
                unsigned int value;
                inputs.checkInput(i, &value);
            }
            waitForInterrupt();
        }
    }


    unsigned int busReturnLogic = bcu.userEeprom->getUInt8(addressStartupDelay - 1) & 0x01;

    for (unsigned int i = 0; i < MAX_LOGIC; i++)
    {
        if (bcu.userEeprom->getUInt8(currentVersion->logicBaseAddress + i * (11 + channels))
                != 0xff)
        {
            logicConfig[i] = new Logic(currentVersion->logicBaseAddress, i,
                    channels, busReturnLogic);
        }
    }

    inputs.scan();
    for (unsigned int i = 0; i < channels; i++)
    {
        unsigned int value;
        int configBase = currentVersion->baseAddress + 4 + i * 46;
        word channelType = bcu.userEeprom->getUInt16(configBase);
        Channel * channel;
        inputs.checkInput(i, &value);
        //leds.setStatus(i, value);
        for (unsigned int n = 0; n < MAX_LOGIC; n++)
        {
            if (logicConfig[n])
            {
                logicConfig[n]->inputChanged(n, value);
            }
        }

        busReturn = bcu.userEeprom->getUInt8(configBase + 32) || bcu.userEeprom->getUInt8(addressStartupDelay - 1) & 0x02; // param sendValue || readToggleObject
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
        case 511: // channel is configured as LED output
        	channel = new LedOutput(i, longKeyTime, configBase, busReturn, value);
            break;
        default:
            channel = 0;
        }
        channelConfig[i] = channel;
    }

}
