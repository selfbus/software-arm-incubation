/*
 *  brigthness_sensor.cpp - The class defining the behavior of one brightness sensor
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#include "brightness_sensor.h"

// send brightness on 0x4511 + number

Brightness_Sensor::Brightness_Sensor()
  : number(0xFF)
  , lastSentBrightness(0)
{
}

static unsigned int timeConversionMinSec(unsigned int address)
{
    unsigned int result = userEeprom.getUInt8(address);
    if (result < 80)
        // the time is in minutes
        result *= 1000*60;
    else
        // the time is kind of seconds (10s -> 80, 20s -> 160, ...)
        result *= 125;
    return result;
}

void Brightness_Sensor::Initialize(unsigned int number)
{
    this->number = number;
    cyclicPeriod =  timeConversionMinSec(0x4514 + number);
    threshold[0].upperLimit     = userEeprom.getUInt8(0x451D + number) * 1000;
    threshold[0].upperLimitTime = timeConversionMinSec(0x452C + number);
    threshold[0].lowerLimit     = userEeprom.getUInt8(0x4520 + number) * 1000;
    threshold[0].lowerLimitTime = timeConversionMinSec(0x452F + number);
    if (userEeprom.getUInt8(0x4523))
    {
        threshold[0].changeLowerLimit = COM_OBJ_THRESHOLD_1_EAST_LOWER + 17 * number;
        threshold[0].changeUpperLimit = COM_OBJ_THRESHOLD_1_EAST_UPPER + 17 * number;
    }
    threshold[0].sendLimitExceeded  = userEeprom.getUInt8(0x4526  + number);
    threshold[0].sendLowerDeviation = userEeprom.getUInt8(0x4529  + number);
    threshold[0].cycleTime = timeConversionMinSec(0x4594 + number);
    if (userEeprom.getUInt8(0x4532))
        threshold[0].blockObjNo = COM_OBJ_BRIGHTNESS_EAST_BLOCK + 17 * number;

    threshold[1].upperLimit     = userEeprom.getUInt8(0x4538 + number) * 1000;
    threshold[1].upperLimitTime = timeConversionMinSec(0x4544 + number);
    threshold[1].lowerLimit     = userEeprom.getUInt8(0x453B + number) * 1000;
    threshold[1].lowerLimitTime = timeConversionMinSec(0x4547 + number);
    threshold[1].sendLimitExceeded  = userEeprom.getUInt8(0x453E  + number);
    threshold[1].sendLowerDeviation = userEeprom.getUInt8(0x4541  + number);
    threshold[1].cycleTime = timeConversionMinSec(0x4597 + number);

    if (userEeprom.getUInt8(0x451A))
        threshold[0].objNumber = COM_OBJ_THRESHOLD_1_EAST + 17 * number;

    if (userEeprom.getUInt8(0x4535))
        threshold[1].objNumber = COM_OBJ_THRESHOLD_2_EAST + 17 * number;
}

void Brightness_Sensor::periodic()
{
    if (number == 255) // this channel is not enabled
        return;
    unsigned int newBrightness = 1; // XXX read new value from sensor

    unsigned int sendMode = userEeprom.getUInt8(0x4511 + number);
    int objNo = COM_OBJ_BRIGHTNESS_EAST + 17 * number;
    if (sendMode & 0x001)
    {   // send on demand -> only if requested from outside
        objNo = -1; // inhibit send of the new value
    }
    if (sendMode & 0x02)
    {   // send on change
        int delta = newBrightness - lastSentBrightness;
        if (delta < 0)
            delta += -1;
        if ((delta * 100 / lastSentBrightness) < userEeprom.getUInt8(0x4517 + number))
            // only send if the changes is more than configured
            objNo = -1;
    }
    if (sendMode & 0x04)
    {   // send cyclic
        if (cyclicSend.stopped() || !cyclicSend.expired())
            // only send if the changes is more than configured
            objNo = -1;
    }
    if (objNo >= 0)
    {
        lastSentBrightness = newBrightness;
        objectWrite(objNo, newBrightness);
        if (sendMode & 0x04)
            cyclicSend.start(cyclicPeriod);
    }
    else
        objectSetValue (objNo, newBrightness);
    unsigned int action_1 = threshold[0].periodic(newBrightness);
    unsigned int action_2 = threshold[1].periodic(newBrightness);
    if (userEeprom.getUInt8(0x450E + number))
    {   // Facade control active
        unsigned int tempReaction = userEeprom.getUInt8(0x45A5 + number);
        unsigned int tempLimit    = userEeprom.getUInt8(0x4574 + number);
        if (  (tempReaction == 0)
           || (  (tempReaction == 1)
              && (temperatur >= tempLimit)
              )
           || (  (tempReaction == 2)
              && (objectRead(COM_OBJ_FACADE_EAST_EXT_TEMP + 17 * number) >= tempLimit)
              )
           )
        {
            switch(userEeprom.getUInt8(0x454D + number))
            {
            case 2: // scene
                if (  (action_1 & IN_UPPER_LIMIT)
                   && (userEeprom.getUInt8(0x4550 + number) == 2)
                   )
                    // the upper limit has been activated
                    objectWrite(COM_OBJ_FACADE_EAST_SCENE + 17 * number, userEeprom.getUInt8(0x4556 + number));
                if (  (action_1 & IN_LOWER_LIMIT)
                   && (userEeprom.getUInt8(0x4568 + number) == 2)
                   )
                    // the lower limit has been activated
                    objectWrite(COM_OBJ_FACADE_EAST_SCENE + 17 * number, userEeprom.getUInt8(0x456B + number));
                if (  (action_2 & IN_UPPER_LIMIT)
                   && (userEeprom.getUInt8(0x4553 + number) == 2)
                   )
                    // the lower limit has been activated
                    objectWrite(COM_OBJ_FACADE_EAST_SCENE + 17 * number, userEeprom.getUInt8(0x4559 + number));
                break;
            case 3: // shutter pos
            case 4: // shutter and slat pos
                if (  (action_1 & IN_UPPER_LIMIT)
                   && (userEeprom.getUInt8(0x4550 + number) == 4)
                   )
                {   // the upper limit has been activated
                    objectWrite(COM_OBJ_FACADE_EAST_POS_SHUTTER + 17 * number, userEeprom.getUInt8(0x4556 + number));
                    objectWrite(COM_OBJ_FACADE_EAST_POS_SLATS   + 17 * number, userEeprom.getUInt8(0x455C + number));
                }
                if (  (action_1 & IN_LOWER_LIMIT)
                   && (userEeprom.getUInt8(0x4568 + number) == 4)
                   )
                {   // the lower limit has been activated
                    objectWrite(COM_OBJ_FACADE_EAST_POS_SHUTTER + 17 * number, userEeprom.getUInt8(0x456B + number));
                    objectWrite(COM_OBJ_FACADE_EAST_POS_SLATS   + 17 * number, userEeprom.getUInt8(0x456E + number));
                }
                if (  (action_2 & IN_UPPER_LIMIT)
                   && (userEeprom.getUInt8(0x4553 + number) == 4)
                   )
                {   // the upper limit has been activated
                    objectWrite(COM_OBJ_FACADE_EAST_POS_SHUTTER + 17 * number, userEeprom.getUInt8(0x4559 + number));
                    objectWrite(COM_OBJ_FACADE_EAST_POS_SLATS   + 17 * number, userEeprom.getUInt8(0x455F + number));
                }
                break;
            }
        }
    }
}

void Brightness_Sensor::objectUpdated(int number)
{
    unsigned int noOff = 17 * this->number;
    if (number == threshold[0].changeLowerLimit)
        // this is the object for changing the lower limit
        threshold[0].lowerLimit = objectRead(number);
    if (number == threshold[0].changeUpperLimit)
        // this is the object for changing the upper limit
        threshold[0].upperLimit = objectRead(number);
    if (  ((number - noOff)         == COM_OBJ_FACADE_EAST_TEACH_IN_P1)
       && (userEeprom.getUInt8(0x4562 + number) == 1)
       )
    {   // a teach request as been received and is enabled for position 1
        userEeprom [0x4556 + number] = objectRead(COM_OBJ_FACADE_EAST_STATUS_POS  + noOff);
        userEeprom [0x455C + number] = objectRead(COM_OBJ_FACADE_EAST_STATUS_SLAT + noOff);
        userEeprom.modified();
    }
    if (  ((number - noOff)         == COM_OBJ_FACADE_EAST_TEACH_IN_P2)
       && (userEeprom.getUInt8(0x4565 + number) == 1)
       )
    {   // a teach request as been received and is enabled for position 2
        userEeprom [0x4559 + number] = objectRead(COM_OBJ_FACADE_EAST_STATUS_POS  + noOff);
        userEeprom [0x455F + number] = objectRead(COM_OBJ_FACADE_EAST_STATUS_SLAT + noOff);
        userEeprom.modified();
    }
}

