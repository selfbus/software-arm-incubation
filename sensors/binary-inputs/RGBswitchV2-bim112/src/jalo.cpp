/*
 *  jalo.cpp -
 *
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <app_RGBswitchV2.h>
#include "jalo.h"

Jalo::Jalo(unsigned int no, unsigned int longPress, unsigned int channelConfig,
        unsigned int busReturn, unsigned int value) :
        _Switch_(no, longPress)
{
    oneButtonShutter = bcu.userEeprom->getUInt8(channelConfig + 0x03) & 0x04 ? 1 : 0;
    shortLongInverse = bcu.userEeprom->getUInt8(channelConfig + 0x03) & 0x02 ? 1 : 0;
    upDownInverse = bcu.userEeprom->getUInt8(channelConfig + 0x03) & 0x01 ? 1 : 0;

    debug_eeprom("Channel EEPROM:", channelConfig, 46);

    if (!oneButtonShutter)
    {
        upDownComObjNo = (number & 0xfffe) * 5;
        stopComObjNo = (number & 0xfffe) * 5 + 1;
        directionComObjNo = -1;
    }
    else
    {
        upDownComObjNo = number * 5;
        stopComObjNo = number * 5 + 1;
        directionComObjNo = number * 5 + 2;
    }
    if (busReturn && oneButtonShutter)
    {
        bcu.comObjects->requestObjectRead(directionComObjNo);
    }
}

void Jalo::inputChanged(int value)
{
    if (value)
    {   // this change is a rising edge, just start the long pressed timeout
        // if a falling edge occurs before the timeout expires
        // the short action will be triggered
        // if the long press timeout expires -> the long press action will be
        // triggered
        timeout.start(longPressTime);
    }
    else
    {   // this change is a falling edge
        // only handle the falling edge if we don't had a long pressed
        // for the last rising edge
        if (timeout.started())
        {
            unsigned int direction = upDownInverse; //use inverse value from parameters
            if (oneButtonShutter)
            { // in one button mode use the inverse direction com obj value
                direction = !bcu.comObjects->objectRead(directionComObjNo);
            }
            if (!shortLongInverse)
            {
                if (oneButtonShutter)
                {
                    direction ^= 1;
                }
                bcu.comObjects->objectWrite(stopComObjNo, direction);
            }
            else
            {
                bcu.comObjects->objectWrite(upDownComObjNo, direction);
            }
        }
        timeout.stop();
    }
}

void Jalo::checkPeriodic(void)
{
    if (timeout.started() && timeout.expired())
    {
        unsigned int direction = upDownInverse; //use inverse value from parameters
        if (oneButtonShutter)
        { // in one button mode use the inverse direction com obj value
            direction = !bcu.comObjects->objectRead(directionComObjNo);
        }
        if (shortLongInverse)
        {
            if (oneButtonShutter)
            {
                direction ^= 1;
            }
            bcu.comObjects->objectWrite(stopComObjNo, direction);
        }
        else
        {
            bcu.comObjects->objectWrite(upDownComObjNo, direction);
        }
    }
}
