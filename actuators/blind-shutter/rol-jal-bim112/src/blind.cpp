/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <blind.h>
#include <sblib/digital_pin.h>
#include <sblib/timer.h>
#include "config.h"

Blind::Blind(unsigned int number, unsigned int address)
  : Channel(number, address)
  , slatMoveForTime(0)
  , slatPosition(0)
  , slatStartPosition(-1)
  , slatTargetPosition(-1)
  , slatSavedPosition(-1)
{
    shortTime = userEeprom.getUInt16(address +   6);
    slatTime  = userEeprom.getUInt16(address +  8);
    for (unsigned int i = 0; i < NO_OF_SCENES; i++)
    {
        sceneSlatPos[i] = userEeprom.getUInt8(address + 24 + i);
    }
    slatPosAfterMove = userEeprom.getUInt8(address + 61);
    for (unsigned int i = 0; i < NO_OF_AUTOMATIC; i++)
    {
        automaticSlatPos[i] = userEeprom.getUInt8(address + 44 + i);
    }
    oneBitSlatPosition = userEeprom.getUInt8(address + 68);
}

void Blind::moveTo(short position)
{
    if (position != this->position)
    {
        Channel::moveTo(position);
        slatSavedPosition = slatPosAfterMove;
    }
    else
        moveSlatTo (slatPosAfterMove);
}

void Blind::moveSlatTo(short position)
{
    if (position != slatPosition)
    {
        slatTargetPosition = position;
        if (targetPosition == -1)
        {
            if (slatTargetPosition < slatPosition)
                startUp();
            else if (slatTargetPosition > slatPosition)
                startDown();
            else
                stop();
        }
        else
            slatSavedPosition = position;
    }
    else
    {
        stop ();
        slatTargetPosition = -1;
    }
}

// XXX check if we need the slatFormTime -> reuse moveForTime???
void Blind::moveSlatFor(unsigned int time, unsigned int direction)
{
    slatMoveForTime = time;
    if (direction == UP)
        startUp();
    else
        startDown();
}

bool Blind::_trackSlatPosition(void)
{
    unsigned int moveBy = timeToPercentage(startTime, slatTime);
    bool slatPosReached = false;
    if (direction == UP)
    {
        slatPosition = slatStartPosition - moveBy;
        if (slatPosition <= 0)
        {
            slatPosition = 0;
            state = MOVE;
            startTime = millis();
        }
        slatPosReached = slatPosition <= slatTargetPosition;
    }
    else
    {
        slatPosition = slatStartPosition + moveBy;
        if (slatPosition >= 255)
        {
            slatPosition = 255;
            state = MOVE;
            startTime = millis();
        }
        slatPosReached = slatPosition >= slatTargetPosition;
    }
    if (  (   slatMoveForTime
          && (elapsed(startTime) >= slatMoveForTime)
          )
       || (  (slatTargetPosition >= 0)
          && (targetPosition == -1)
          && slatPosReached
          )
       )
    {
        slatMoveForTime = 0;
        slatTargetPosition = -1;
        stop ();
    }
    return false;
}

void Blind::_savePosition(bool currentPosition)
{
    Channel::_savePosition(currentPosition);
    if (currentPosition)
         slatSavedPosition = slatPosition;
    else slatSavedPosition = -1;
}

void Blind::_moveToAutomaticPosition(unsigned int pos)
{
    Channel::_moveToAutomaticPosition(pos);
    slatAutoPosition = automaticSlatPos [pos];
    moveSlatTo (slatAutoPosition);
}

bool Blind::_inSavedPosition(void)
{
    return (position == savedPosition) && (slatPosition == slatSavedPosition);
}

void Blind::_startTracking(void)
{
    slatStartPosition = slatPosition;
    Channel::_startTracking();
}

bool Blind::_storeScene(unsigned int i)
{
    bool result = Channel::_storeScene(i);
    unsigned int address = currentVersion->baseAddress + EE_CHANNEL_CFG_SIZE * number;
    sceneSlatPos [i] = slatPosition;
    if (userEeprom [address + 24 + i] != slatPosition)
    {
        userEeprom [address + 24 + i] = slatPosition;
        result = true;
    }
    return result;
}

void Blind::_moveToScene(unsigned int i)
{
    moveTo     (scenePos     [i]);
    moveSlatTo (sceneSlatPos [i]);
}

// XXX
bool Blind::_restorePosition(void)
{
    bool result = Channel::_restorePosition();
    if (  ! result && (slatSavedPosition != -1)
       )
    {
        moveSlatTo(slatSavedPosition);
        slatSavedPosition = -1;
        result = true;
    }
    return result;
}
