/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "config.h"
#include "channel.h"
#include <sblib/digital_pin.h>
#include <sblib/io_pin_names.h>
#include <sblib/timer.h>
#include <sblib/eib.h>
#include <string.h>


const int outputPins[NO_OF_OUTPUTS] =
    { PIN_IO1, PIN_IO2, PIN_IO3, PIN_IO4, PIN_IO5, PIN_IO6, PIN_IO7, PIN_IO8 };

Timeout PWMDisabled;

Channel::Channel(unsigned int number, unsigned int address)
  : shortTime(0)
  , number(number)
  , firstObjNo(13 + number * 20)
  , positionValid(false)
  , features(0)
  , limits(0)
  , state(PROTECT)
  , direction(STOP)
  , moveForTime(0)
  , startTime(0)
  , position(0)
  , startPosition(-1)
  , targetPosition(-1)
  , savedPosition(-1)
{
    for (unsigned int i = number * 2 ;i <= (number * 2 + 1); i++)
    {
        pinMode(outputPins [i], OUTPUT);
        digitalWrite(outputPins [i], 0);
    }

    pauseChangeDir = userEeprom.getUInt16 (address +   2);
    openTime       = userEeprom.getUInt16 (address +   4) * 1000;
    minMoveTime    = userEeprom.getUInt8  (address +  10);
    for (unsigned int i = 0; i < NO_OF_SCENES; i++)
    {
        scenePos[i]    = userEeprom.getUInt8(address + 16 + i);
        sceneNumber[i] = userEeprom.getUInt8(address + 32 + i);
    }
    for (unsigned int i = 0; i < NO_OF_AUTOMATIC; i++)
    {
        automaticPos[i] = userEeprom.getUInt8(address + 40 + i);
    }
    _enableFeature(address + 48,                FEATURE_STORE_SCENE);
    //_enableFeature(address + 49,                FEATURE_AUTOMATIK);
    //_enableFeature(address + 50,                FEATURE_MANUELL);
    //_enableFeature(address + 51,                FEATURE_ABS_POSITION, 0x04);
    lockConfig     = userEeprom.getUInt8  (address +  52);
    topLimitPos    = userEeprom.getUInt8  (address +  53);
    botLimitPos    = userEeprom.getUInt8  (address +  54);
    motorOnDelay   = userEeprom.getUInt8  (address +  55);
    motorOffDelay  = userEeprom.getUInt8  (address +  56);
    // userEeprom.getUInt8  (address +  57); // ?? stop Mode
    unsigned char extMoveTime    = userEeprom.getUInt8 (address +  58);
    _enableFeature(address +  59, FEATURE_RESTORE_AFTER_REF);
    reactionLock   = userEeprom.getUInt8  (address +  60);
    closeTime      = userEeprom.getUInt16 (address +  62) * 1000;
    if (closeTime == 0)
        closeTime = openTime;
    unsigned char lockAbsPos     = userEeprom.getUInt8  (address +  64);
    if (lockAbsPos & 0x80)
        lockConfig |= LOCK_POS_UP_DOWN;
    if (lockAbsPos & 0x20)
        lockConfig |= LOCK_POS_RELEASE_UP;
    _enableFeature(address + 65,                FEATURE_STATUS_MOVING,   0x01);
    _enableFeature(address + 65,                FEATURE_SHORT_OPERATION, 0x02);
    obj24Config    = userEeprom.getUInt8(address + 66);
    oneBitPosition = userEeprom.getUInt8(address + 67);

    openTimeExt    = openTime  * extMoveTime / 100;
    closeTimeExt   = closeTime * extMoveTime / 100;
    unsigned int baseAddr =
          currentVersion->baseAddress
        + currentVersion->noOfChannels * EE_CHANNEL_CFG_SIZE
        + (EE_ALARM_HEADER_SIZE + EE_ALARM_CFG_SIZE * NO_OF_ALARMS) * number;

    baseAddr += EE_ALARM_HEADER_SIZE;
    for (unsigned int i = 0; i < NO_OF_ALARMS; i++, baseAddr += EE_ALARM_CFG_SIZE)
    {
        alarm[i].monitorTime   = userEeprom.getUInt16 (baseAddr + 0);
        alarm[i].priority      = userEeprom.getUInt16 (baseAddr + 2);
        alarm[i].releaseAction = userEeprom.getUInt8  (baseAddr + 5);
        alarm[i].engageAction  = userEeprom.getUInt8  (baseAddr + 6);
        if (userEeprom.getUInt8 (baseAddr + 4) == 255)
            alarm[i].priority  = 0;
    }
    baseAddr = currentVersion->baseAddress
             + currentVersion->noOfChannels
             * (EE_CHANNEL_CFG_SIZE + EE_ALARM_HEADER_SIZE + EE_ALARM_CFG_SIZE * NO_OF_ALARMS);
    busDownAction   = userEeprom.getUInt8 (baseAddr + 0x00 + 2 * number);
    busReturnAction = userEeprom.getUInt8 (baseAddr + 0x01 + 2 * number);
    autoConfig      = userEeprom.getUInt8 (baseAddr + 0x10 + number);
    if (autoConfig != 0xFF) // automatic for this channel is enabled
    {
        if (autoConfig & 0x80)
             features |= FEATURE_AUTOMATIC_B;
        else features |= FEATURE_AUTOMATIC_A;
    }
    _enableFeature(baseAddr + 0x18 + number, FEATURE_CENTRAL);
    timeout.start (pauseChangeDir);
}

void Channel::objectUpdate(unsigned int objno)
{
    unsigned int fct = objno - firstObjNo;
    unsigned int value = objectRead(objno);
    switch (fct)
    {
    case 0 : // object for move Up/Down (long)
        handleMove(value);
        break;
    case 1 : // object for Stop, slat open/close (short)
        handleStep(value);
        break;
    case 2: // object for stopping the movement
        stop();
        break;
    case 3: // object for scene
        handleScene(value);
        break;
    case 5: // object for requested position
        moveTo(value);
        break;
    case 6: // object for requested slat position
        break;
    case 10: // object for start reference drive
        _savePosition (features & FEATURE_RESTORE_AFTER_REF);
        moveTo        (0);
        break;
    case 11: // object for move to limit (1 bit)/move to position (1 bit)
        // TODO: implement one bit positioning
        break;
    case 14: // object for lock absolute position
        // TODO: implement lock absolute positioning
        break;
    case 15: // object for lock universal
        // TODO: implement lock universal
        break;
    case 16: // object for wind alert
        // TODO: implement wind alert
        break;
    case 17: // object for rain alert
        // TODO: implement rain alert
       break;
    case 18: // object for frost alert
        // TODO: implement frost alert
        break;
    case 19: // object for lock
        // TODO: implement lock
        break;
        // the following objects are send only
    case 4: // object for state of movement
    case 7: // object for status current position
    case 8: // object for status current slat position
    case 9: // object for position valid
    case 12: // object for status top position
    case 13: // object for status bottom position
    default:
        break;
    }
}

unsigned int Channel::timeToPercentage(unsigned int startTime, unsigned int maxTime)
{
    unsigned int diff = millis() - startTime;
    return (diff * 256) / maxTime;
}

void Channel::startUp(void)
{
    if (direction == DOWN)
        stop();
    if (  (direction != UP)
       && (!_isInPosition (0) || !positionValid)
       )
    {
        direction = UP;
    }
}

void Channel::startDown(void)
{
    if (direction == UP)
        stop();
    if (  (direction != DOWN)
       && (!_isInPosition(255) || !positionValid)
       )
    {
        direction = DOWN;
    }
}

void Channel::stop(void)
{
    direction = STOP;
    if (state & MOVE)
    {
        digitalWrite(outputPins[number * 2 + 0], 0);
        digitalWrite(outputPins[number * 2 + 1], 0);
#ifdef HAND_ACTUATION
        handAct.setLedState(number * 2 + 0, 0);
        handAct.setLedState(number * 2 + 1, 0);
#endif
        state     = PROTECT;
        timeout.start(pauseChangeDir);
    }
}

void Channel::_handleState(void)
{
    switch(state)
    {
    case DELAY:
        if (timeout.expired ())
        {
            _startTracking();
            state = SLAT_MOVE;
        }
        break;
    case PROTECT:
        if (timeout.expired ())
            state = IDLE;
        break;
    case SLAT_MOVE:
        // if trackSlatPosition returns true we want to start tracking the position
        if (! _trackSlatPosition())
            break;
    case MOVE:
        _trackPosition();
        break;
    case EXTEND:
        if (timeout.expired())
        {
            positionValid = true;
            targetPosition = -1;
            if (! _restorePosition()) // check if we need to restore a saved position
            {
                stop ();
            }
            else
            {
                state     = PROTECT;
                timeout.start(pauseChangeDir);
            }
        }
        break;
    default:
    case IDLE:
        if (direction != STOP)
        {
            unsigned int outNo = number * 2;
            if (direction == DOWN) outNo++;
            digitalWrite(outputPins[outNo], 1);
            timer16_0.match(MAT2, PWM_PERIOD);// disable the PWM
            PWMDisabled.start(PWM_TIMEOUT);
#ifdef HAND_ACTUATION
            handAct.setLedState(outNo, 1);
#endif
            timeout.start(motorOnDelay);
            state = DELAY;
        }
        break;
    }
}

bool Channel::_trackPosition(void)
{
    // We assume that the movement has started motorOffDelay earlier to achive the current
    // position when we stop the motor
    int moveBy;
    if (direction == UP)
         moveBy = -1 * timeToPercentage(startTime - motorOffDelay, openTime);
    else moveBy =      timeToPercentage(startTime - motorOffDelay, closeTime);
    position    = startPosition + moveBy;

    if ((position <= 0) && (direction == UP))
    {
        position = 0;
        state = EXTEND;
        timeout.start(openTimeExt);
    }
    if ((position >= 255) && (direction == DOWN))
    {
        position = 255;
        state = EXTEND;
        timeout.start(openTimeExt);
    }

    // check if we moved for a requested time
    if (  (   moveForTime
          && (elapsed(startTime) >= moveForTime)
          )
        )
    {
        moveForTime = 0;
        stop();
    }
    // check if an absolute position has been reached
    if (  (targetPosition >= 0)
       && (position == targetPosition)
       && (state != EXTEND)
       )
    {
        targetPosition = -1; // clear the target position
        if (! _restorePosition()) // check if we need to restore a saved position
        {
            stop ();
        }
    }
    _updatePosState(position <= topLimitPos, IN_TOP_POSITION, 12);
    _updatePosState(position >= botLimitPos, IN_BOT_POSITION, 13);
    return false;
}

bool Channel::_trackSlatPosition(void)
{
    state = MOVE;
    return true;
}

void Channel::periodic(void)
{
    _handleState();
}

void Channel::_updatePosState(unsigned int current, unsigned int mask, unsigned int objno)
{
    objno += firstObjNo;
    if ((limits & mask) && !current)
    {
        limits &= ~mask;
        objectWrite(objno, (unsigned int) 0);
    }
    if (!(limits & mask) && current)
    {
        limits |= mask;
        objectWrite(objno, (unsigned int) 1);
    }
}

void Channel::moveTo(short position)
{
    targetPosition = position;
    if (targetPosition < this->position)
    {
        startUp();
    }
    else if (targetPosition > this->position)
    {
        startDown();
    }
    else
    {
        stop ();
        targetPosition = -1;
    }
}

void Channel::moveFor(unsigned int time, unsigned int direction)
{
    moveForTime = time;
    if (direction == UP)
        startUp();
    else
        startDown();
}

// XXX check config values
void Channel::handleAutomaticFunction(unsigned int pos, unsigned int block, unsigned int value)
{
    if (  (!block && automaticAEnabled())
       || ( block && automaticBEnabled())
       )
    {
        if (value)
        {   // automatic position should be activated
            bool engage = !(autoConfig & 0x03)
                       || ((autoConfig & 0x01) && (position == 255))
                       || ((autoConfig & 0x02) && (position ==   0));
            if (engage)
            {
                if (autoConfig & 0x70)
                {
                    switch (block)
                    {
                    case 0 : engage = (autoConfig & 0x70) == 0x10; break;
                    case 1 : engage = (autoConfig & 0x70) == 0x30; break;
                    case 2 : engage = (autoConfig & 0x70) == 0x50; break;
                    case 3 : engage = (autoConfig & 0x70) == 0x70; break;
                    }
                }
                if (engage)
                {
                    _moveToAutomaticPosition(pos);
                }
            }
        }
        else if (_stillInAutoPosition())
        {   // handle return from automatic only if the position has
            // not been changed
            if (autoConfig & 0x08)
                startUp ();
            else if (autoConfig & 0x04)
                startDown ();
        }
    }
}

void Channel::_moveToAutomaticPosition(unsigned int pos)
{
    autoPosition = automaticPos[pos];
    moveTo (autoPosition);
}

bool Channel::_inSavedPosition(void)
{
    return position == savedPosition;
}

void Channel::handleMove(unsigned int value)
{
    if (value)
        startDown();
    else
        startUp();
}

void Channel::handleStep(unsigned int value)
{
}

void Channel::handleScene(unsigned int value)
{
    bool save = value & 0x80;
    value &= 0x3F;

    for (unsigned int i = 0; i < NO_OF_SCENES; i++)
    {
        if (sceneNumber [i] == value)
        {
            if (save)
            {
                if (features & FEATURE_STORE_SCENE)
                {
                    if (_storeScene(i))
                        userEeprom.modified();
               }
            }
            else
            {
                _moveToScene(i);
            }
        }
    }
}

void Channel::_savePosition(bool currentPosition)
{
    if (currentPosition)
         savedPosition = position;
    else savedPosition = -1;
}

void Channel::_startTracking(void)
{
     startPosition = position;
     startTime = millis();
}

bool Channel::_storeScene(unsigned int i)
{
    unsigned int address = currentVersion->baseAddress + EE_CHANNEL_CFG_SIZE * number;
    scenePos [i] = position;
    if (userEeprom [address + 32 + i] != position)
    {
        userEeprom [address + 32 + i] = position;
        return true;
    }
    return false;
}

void Channel::_moveToScene(unsigned int i)
{
    moveTo     (scenePos [i]);
}

bool Channel::_restorePosition(void)
{
    if (savedPosition != -1)
    {
        moveTo (savedPosition);
        savedPosition = -1;
        return true;
    }
    return false;
}

void Channel::_enableFeature(unsigned int address, unsigned int feature, unsigned int mask)
{
    if (userEeprom.getUInt8(address) & mask)
    {
        features |= feature;
    }
}


