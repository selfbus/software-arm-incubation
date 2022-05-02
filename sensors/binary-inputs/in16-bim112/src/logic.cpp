/*
 * logic.cpp
 *
 *  Created on: 08.08.2015
 *      Author: Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 */

#include "logic.h"
#include "app_in.h"

Logic::Logic(unsigned int logicBase, unsigned int no, unsigned int chans,
        unsigned int busreturn) :
        configBase(logicBase), number(no), channels(chans)
{
    unsigned int base = configBase + number * (11 + channels);
    logicOperation = bcu.userEeprom->getUInt8(base);
    sendCondition = bcu.userEeprom->getUInt8(base + 1);
    outputInverted = bcu.userEeprom->getUInt8(base + 2);
    inputCfgPtr = base + 3;

    int addr = configBase + 4 * (11 + channels) + 1 + 2 * number;
    numScene = bcu.userEeprom->getUInt8(addr + 1);
    for (unsigned int i = 0; i < MAX_CHANNELS + 2; i++)
    {
        inputs[i] = 0;
    }

    debug_eeprom("Logic-Channel:", base, 11 + channels);
    debug_eeprom("Scene Values:", addr, 2);

    int logicObjStart = channels/2 *10;
    extLogicalObjectAComObjNo = logicObjStart + (number * 3);
    extLogicalObjectBComObjNo = logicObjStart + 1 + (number * 3);
    outLogicalObjectComObjNo = logicObjStart + 2 + (number * 3);

    if (busreturn)
    {
        bcu.comObjects->requestObjectRead(extLogicalObjectAComObjNo);
        bcu.comObjects->requestObjectRead(extLogicalObjectBComObjNo);
    }
}

void Logic::doLogic(void)
{
    unsigned int result = 0;
    unsigned int chanModePtr = 0;

    if (logicOperation == LOGIC_OP_OR)
    {
        for (unsigned int i = 0; i < channels + 2; i++)
        {
            if (i < channels)
            {
            	chanModePtr = inputCfgPtr + i;
            }
            else
            {
            	chanModePtr = inputCfgPtr + i + 2*number;
            }
        	unsigned int chanMode = bcu.userEeprom->getUInt8(chanModePtr);
            if (chanMode != CHAN_MODE_DISABLED)
            {
                result |= inputs[i] ^ (chanMode - 1);
            }
        }
    }
    else if (logicOperation == LOGIC_OP_AND)
    {
        result = 1;
        for (unsigned int i = 0; i < channels + 2; i++)
        {
        	if (i < channels)
        	{
        	   	chanModePtr = inputCfgPtr + i;
        	}
        	else
        	{
        	   	chanModePtr = inputCfgPtr + i + 2*number;
        	}
        	unsigned int chanMode = bcu.userEeprom->getUInt8(chanModePtr);
            if (chanMode != CHAN_MODE_DISABLED)
            {
                result &= inputs[i] ^ (chanMode - 1);
            }
        }
    }
    if (outputInverted)
    {
        result ^= 1;
    }
    switch (sendCondition)
    {
    case SEND_COND_INPUT:
        bcu.comObjects->objectWrite(outLogicalObjectComObjNo, result);
        break;
    case SEND_COND_OUTPUT:
        if (bcu.comObjects->objectRead(outLogicalObjectComObjNo) != result)
        {
            bcu.comObjects->objectWrite(outLogicalObjectComObjNo, result);
        }
        ;
        break;
    case SEND_COND_SCENE:
        if (result)
        {
            bcu.comObjects->objectWrite(outLogicalObjectComObjNo, numScene);
        }
        ;
        break;
    case SEND_COND_NONE:
        bcu.comObjects->objectSetValue(outLogicalObjectComObjNo, result);
        break;
    }
}

void Logic::inputChanged(int num, int value)
{
    unsigned int chanMode = bcu.userEeprom->getUInt8(inputCfgPtr + num);
    if (chanMode != CHAN_MODE_DISABLED)
    {
        inputs[num] = value ? 1 : 0;
        doLogic();
    }
}

void Logic::objectUpdated(int objno)
{
    if (objno == extLogicalObjectAComObjNo)
    {
        unsigned int chanMode = bcu.userEeprom->getUInt8(inputCfgPtr + channels + 2*number);
        if (chanMode != CHAN_MODE_DISABLED)
        {
            inputs[channels] = bcu.comObjects->objectRead(extLogicalObjectAComObjNo);
            doLogic();
        }
    }
    if (objno == extLogicalObjectBComObjNo)
    {
        unsigned int chanMode = bcu.userEeprom->getUInt8(inputCfgPtr + channels + 1 + 2*number);
        if (chanMode != CHAN_MODE_DISABLED)
        {
            inputs[channels+1] = bcu.comObjects->objectRead(extLogicalObjectBComObjNo);
            doLogic();
        }
    }
}

