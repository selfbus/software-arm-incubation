/*
 *  dimmer.cpp -
 *
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "dimmer.h"
#include "app_in.h"

Dimmer::Dimmer(unsigned int no, unsigned int longPress,
        unsigned int channelConfig, unsigned int busReturn, unsigned int value) :
        _Switch_(no, longPress)
{
    oneButtonDimmer = bcu.userEeprom->getUInt8(channelConfig + 0x03) & 0x03 ? 0 : 1;
    upDownInverse = bcu.userEeprom->getUInt8(channelConfig + 0x03) & 0x03;
    stepsDownWidthDimmer = bcu.userEeprom->getUInt8(channelConfig + 0x04);
    stepsUpWidthDimmer = bcu.userEeprom->getUInt8(channelConfig + 0x08);
    repeatDimmerTime = bcu.userEeprom->getUInt16(channelConfig + 0x1E);

    debug_eeprom("Channel EEPROM:", channelConfig, 46);

    if (!(bcu.userEeprom->getUInt8(channelConfig + 0x20) & 0x3))
    {
        repeatDimmerTime = 0;
    }
    doStopFlag = 0;

    if (!oneButtonDimmer)
    {
        onOffComObjNo = (number & 0xfffe) * 5;
        dimValComObjNo = (number & 0xfffe) * 5 + 1;
        stateComObjNo = -1;
        if (number & 0x01)
        { // patch this value due to wrong knxprod file
            stepsDownWidthDimmer = 9;
            if (upDownInverse == 1)
            {
                upDownInverse = 0;
            }
        }
        else
        {
            if (upDownInverse == 2)
            {
                upDownInverse = 0;
            }
        }
    }
    else
    {
        onOffComObjNo = number * 5;
        dimValComObjNo = number * 5 + 1;
        stateComObjNo = number * 5 + 2;
    }
    if (busReturn && oneButtonDimmer)
    {
        bcu.comObjects->requestObjectRead(stateComObjNo);
    }
}

void Dimmer::inputChanged(int value)
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
        if (doStopFlag)
        {
            int objVal = 0;
            doStopFlag = 0;
            bcu.comObjects->objectWrite(dimValComObjNo, objVal);
            if (oneButtonDimmer)
            {
                upDownInverse = !upDownInverse;
            }
        }
        else if (timeout.started())
        {
            unsigned int state = !upDownInverse;
            if (oneButtonDimmer)
            { // in one button mode use the inverse direction com obj value
                state = !bcu.comObjects->objectRead(stateComObjNo);
                bcu.comObjects->objectSetValue(stateComObjNo, state);
            }
            else
            {
                state = upDownInverse ? (number & 1) : !(number & 1);
            }
            bcu.comObjects->objectWrite(onOffComObjNo, state);
        }
        timeout.stop();
    }
}

void Dimmer::checkPeriodic(void)
{
    if (timeout.started() && timeout.expired())
    {
        int val = upDownInverse ? stepsDownWidthDimmer : stepsUpWidthDimmer;
        if (oneButtonDimmer)
        {
            if (repeatDimmerTime)
            {
                timeout.start(repeatDimmerTime);
            }
        }
        bcu.comObjects->objectWrite(dimValComObjNo, val);
        doStopFlag = 1;
    }
}
