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

BrightnessSensor::BrightnessSensor()
  : number(0xFF)
  , objNoOffset(17 * number)
{
}

void BrightnessSensor::Initialize(unsigned int number)
{
    this->number = number;
    InitializeSensor
        ( userEeprom.getUInt8(0x4511 + number)
        , COM_OBJ_BRIGHTNESS_EAST + objNoOffset
        , 0x4514 + number
        , userEeprom.getUInt8(0x4517 + number)
        );
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
        // check if threshold 1 is active
        threshold[0].objNumber = COM_OBJ_THRESHOLD_1_EAST + 17 * number;

    if (userEeprom.getUInt8(0x4535))
        // check if threshold 2 is active
        threshold[1].objNumber = COM_OBJ_THRESHOLD_2_EAST + 17 * number;
}

void BrightnessSensor::periodic(unsigned int temperature)
{
    if (number == 255) // this channel is not enabled
        return;
    unsigned int newBrightness = _newValue();

    _handleNewValue(newBrightness);

    threshold[0].periodic(newBrightness);
    threshold[1].periodic(newBrightness);
    if (userEeprom.getUInt8(0x450E + number))
    {
        _handleFacade(newBrightness, temperature);
    }
}

void BrightnessSensor::_handleFacade(unsigned int newBrightness, unsigned int temperature)
{
    unsigned int tempReaction = userEeprom.getUInt8(0x45A5 + number);
    unsigned int tempLimit    = userEeprom.getUInt8(0x4574 + number);
    bool th1_in_up            = threshold[0].upperLimitJustExceeded();
    bool th1_in_lo            = threshold[0].lowerLimitJustUndershot();
    bool th2_in_up            = threshold[1].upperLimitJustExceeded();

    if (  (tempReaction == 0)
       || (  (tempReaction == 1)
          && (temperature >= tempLimit)
          )
       || (  (tempReaction == 2)
          && (objectRead(COM_OBJ_FACADE_EAST_EXT_TEMP + objNoOffset) >= tempLimit)
          )
       )
    {
        switch(userEeprom.getUInt8(0x454D + number))
        {
        case 2: // scene
            if (  th1_in_up
               && (userEeprom.getUInt8(0x4550 + number) == 2)
               )
                // the upper limit has been activated
                objectWrite(COM_OBJ_FACADE_EAST_SCENE + objNoOffset, userEeprom.getUInt8(0x4556 + number));
            if (  th1_in_lo
               && (userEeprom.getUInt8(0x4568 + number) == 2)
               )
                // the lower limit has been activated
                objectWrite(COM_OBJ_FACADE_EAST_SCENE + objNoOffset, userEeprom.getUInt8(0x456B + number));
            if (  th2_in_up
               && (userEeprom.getUInt8(0x4553 + number) == 2)
               )
                // the lower limit has been activated
                objectWrite(COM_OBJ_FACADE_EAST_SCENE + objNoOffset, userEeprom.getUInt8(0x4559 + number));
            break;
        case 3: // shutter pos
        case 4: // shutter and slat pos
            if (  th1_in_up
               && (userEeprom.getUInt8(0x4550 + number) == 4)
               )
            {   // the upper limit has been activated
                objectWrite(COM_OBJ_FACADE_EAST_POS_SHUTTER + objNoOffset, userEeprom.getUInt8(0x4556 + number));
                objectWrite(COM_OBJ_FACADE_EAST_POS_SLATS   + objNoOffset, userEeprom.getUInt8(0x455C + number));
            }
            if (  th1_in_lo
               && (userEeprom.getUInt8(0x4568 + number) == 4)
               )
            {   // the lower limit has been activated
                objectWrite(COM_OBJ_FACADE_EAST_POS_SHUTTER + objNoOffset, userEeprom.getUInt8(0x456B + number));
                objectWrite(COM_OBJ_FACADE_EAST_POS_SLATS   + objNoOffset, userEeprom.getUInt8(0x456E + number));
            }
            if (  th2_in_up
               && (userEeprom.getUInt8(0x4553 + number) == 4)
               )
            {   // the upper limit has been activated
                objectWrite(COM_OBJ_FACADE_EAST_POS_SHUTTER + objNoOffset, userEeprom.getUInt8(0x4559 + number));
                objectWrite(COM_OBJ_FACADE_EAST_POS_SLATS   + objNoOffset, userEeprom.getUInt8(0x455F + number));
            }
            break;
        }
    }
}

void BrightnessSensor::objectUpdated(int number)
{
    if (number == threshold[0].changeLowerLimit)
        // this is the object for changing the lower limit
        threshold[0].lowerLimit = objectRead(number);
    if (number == threshold[0].changeUpperLimit)
        // this is the object for changing the upper limit
        threshold[0].upperLimit = objectRead(number);
    if (  ((number - objNoOffset)         == COM_OBJ_FACADE_EAST_TEACH_IN_P1)
       && (userEeprom.getUInt8(0x4562 + number) == 1)
       )
    {   // a teach request as been received and is enabled for position 1
        userEeprom [0x4556 + number] = objectRead(COM_OBJ_FACADE_EAST_STATUS_POS  + objNoOffset);
        userEeprom [0x455C + number] = objectRead(COM_OBJ_FACADE_EAST_STATUS_SLAT + objNoOffset);
        userEeprom.modified();
    }
    if (  ((number - objNoOffset)         == COM_OBJ_FACADE_EAST_TEACH_IN_P2)
       && (userEeprom.getUInt8(0x4565 + number) == 1)
       )
    {   // a teach request as been received and is enabled for position 2
        userEeprom [0x4559 + number] = objectRead(COM_OBJ_FACADE_EAST_STATUS_POS  + objNoOffset);
        userEeprom [0x455F + number] = objectRead(COM_OBJ_FACADE_EAST_STATUS_SLAT + objNoOffset);
        userEeprom.modified();
    }
}

// XXX implement
unsigned int BrightnessSensor::_newValue()
{
    return 1;
}
