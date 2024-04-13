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
#include <string.h>
#include <blind.h> ///\todo Blind is a child class of Channel, something is here wrong

MASK0701 bcu = MASK0701();
Timeout PWMDisabled;



/*
 *  sets PWM Frequency and pulse width (more or less copy & paste from out8-bcu1 (outputs.cpp)
 */
void Channel::initPWM(int PWMPin)
{
    pinMode(PWMPin, OUTPUT_MATCH);       // configure digital pin PWMPin to match MAT2 of PWMTimer
    TIMER_PWM.begin();
    TIMER_PWM.prescaler((SystemCoreClock / 100000) - 1);
    TIMER_PWM.matchMode(MAT2, SET);      // set the output of PWMPin to 1 when the timer matches MAT2
    TIMER_PWM.match(MAT2, PWM_DUTY);     // match MAT2 when the timer reaches this value
    TIMER_PWM.pwmEnable(MAT2);           // enable PWM for match channel MAT2

    TIMER_PWM.matchMode(MAT3, RESET);    // Reset the timer when the timer matches MAT3
    TIMER_PWM.match(MAT3, PWM_PERIOD);   // match MAT3 to create 14lHz

    Channel::startPWM();                 // start PWM
}

/*
 *  start PWM with PWM_DUTY pulse width
 */
void Channel::startPWM()
{
    TIMER_PWM.match(MAT2, PWM_DUTY);  // match MAT2 when the timer reaches this value
    TIMER_PWM.pwmEnable(MAT2);        // enable PWM for match channel MAT2
    TIMER_PWM.start();
}

/*
 *  "stops" PWM by setting pulse width to PWM_DUTY_MAX
 */
void Channel::setPWMtoMaxDuty()
{
    TIMER_PWM.match(MAT2, PWM_DUTY_MAX);  // match MAT2 when the timer reaches this value
}

Channel::Channel(unsigned int number, unsigned int address, short position)
  : shortTime(0)
  , number(number)
  , firstObjNo(13 + number * 20)
  , positionValid(false)
  , features(0)
  , limits(0)
  , activeAlarms(0)
  , activeLocks(0)
  , state(PROTECT)
  , direction(STOP)
  , moveForTime(0)
  , startTime(0)
  , position(position)
  , startPosition(-1)
  , targetPosition(-1)
  , savedPosition(-1)
  , handAct_(nullptr)
{
    for (unsigned int i = number * 2 ;i <= (number * 2 + 1); i++)
    {
        pinMode(outputPins [i], OUTPUT);
        switchOutputPin(outputPins [i], OUTPUT_LOW);
    }

    pauseChangeDir = bcu.userEeprom->getUInt16 (address +   2);
    openTime       = bcu.userEeprom->getUInt16 (address +   4) * 1000;
    minMoveTime    = bcu.userEeprom->getUInt8  (address +  10);
    for (unsigned int i = 0; i < NO_OF_SCENES; i++)
    {
        scenePos[i]    = bcu.userEeprom->getUInt8(address + 16 + i);
        sceneNumber[i] = bcu.userEeprom->getUInt8(address + 32 + i);
    }
    for (unsigned int i = 0; i < NO_OF_AUTOMATIC; i++)
    {
        automaticPos[i] = bcu.userEeprom->getUInt8(address + 40 + i);
    }
    _enableFeature(address + 48,                FEATURE_STORE_SCENE);
    //_enableFeature(address + 49,                FEATURE_AUTOMATIK);
    //_enableFeature(address + 50,                FEATURE_MANUELL);
    //_enableFeature(address + 51,                FEATURE_ABS_POSITION, 0x04);
    lockConfig     = bcu.userEeprom->getUInt8  (address +  52);
    topLimitPos    = bcu.userEeprom->getUInt8  (address +  53);
    botLimitPos    = bcu.userEeprom->getUInt8  (address +  54);
    motorOnDelay   = bcu.userEeprom->getUInt8  (address +  55);
    motorOffDelay  = bcu.userEeprom->getUInt8  (address +  56);
    // userEeprom.getUInt8  (address +  57); // ?? stop Mode
    unsigned char extMoveTime    = bcu.userEeprom->getUInt8 (address +  58);
    _enableFeature(address +  59, FEATURE_RESTORE_AFTER_REF);
    closeTime      = bcu.userEeprom->getUInt16 (address +  62) * 1000;
    if (closeTime == 0)
        closeTime = openTime;
    unsigned char lockAbsPos     = bcu.userEeprom->getUInt8  (address +  64);
    if (lockAbsPos & 0x80)
        lockConfig |= LOCK_POS_UP_DOWN;
    if (lockAbsPos & 0x20)
        lockConfig |= LOCK_POS_RELEASE_UP;
    _enableFeature(address + 65,                FEATURE_STATUS_MOVING,   0x80);
    _enableFeature(address + 65,                FEATURE_SHORT_OPERATION, 0x40);
    obj24Config    = bcu.userEeprom->getUInt8(address + 66);
    oneBitPosition = bcu.userEeprom->getUInt8(address + 67);

    if (extMoveTime != 0)
    {
        openTimeExt    = openTime  * extMoveTime / 100;
        closeTimeExt   = closeTime * extMoveTime / 100;
    }
    else
    {
        openTimeExt    = 1;
        closeTimeExt   = 1;
    }
    unsigned int baseAddr =
          currentVersion.baseAddress
        + currentVersion.noOfChannels * EE_CHANNEL_CFG_SIZE
        + (EE_ALARM_HEADER_SIZE + EE_ALARM_CFG_SIZE * NO_OF_ALARMS) * number;

    reactionLockRemove = bcu.userEeprom->getUInt8  (baseAddr +  8);
    baseAddr += EE_ALARM_HEADER_SIZE;
    for (unsigned int i = 0; i < NO_OF_ALARMS; i++, baseAddr += EE_ALARM_CFG_SIZE)
    {
        alarms[i].monitorTime   = bcu.userEeprom->getUInt16 (baseAddr + 0);
        alarms[i].priority      = bcu.userEeprom->getUInt16 (baseAddr + 2);
        alarms[i].releaseAction = bcu.userEeprom->getUInt8  (baseAddr + 5);
        alarms[i].engageAction  = bcu.userEeprom->getUInt8  (baseAddr + 6);
        if (bcu.userEeprom->getUInt8 (baseAddr + 4) == 255)
            alarms[i].priority  = 0;
    }
    baseAddr = currentVersion.baseAddress
             + currentVersion.noOfChannels
             * (EE_CHANNEL_CFG_SIZE + EE_ALARM_HEADER_SIZE + EE_ALARM_CFG_SIZE * NO_OF_ALARMS);
    busDownAction   = bcu.userEeprom->getUInt8 (baseAddr + 0x00 + 2 * number);
    busReturnAction = bcu.userEeprom->getUInt8 (baseAddr + 0x01 + 2 * number);
    autoConfig      = bcu.userEeprom->getUInt8 (baseAddr + 0x10 + number);
    if (autoConfig != 0xFF) // automatic for this channel is enabled
    {
        if (autoConfig & 0x80)
             features |= FEATURE_AUTOMATIC_B;
        else features |= FEATURE_AUTOMATIC_A;
    }
    _enableFeature(baseAddr + 0x18 + number, FEATURE_CENTRAL);
    timeout.start (pauseChangeDir);
    bcu.comObjects->objectSetValue(firstObjNo + COM_OBJ_POSITION, position);
#ifdef BUSFAIL
    bcu.comObjects->objectSetValue(firstObjNo + COM_OBJ_POS_VALID, 1);
#else
    bcu.comObjects->objectSetValue(firstObjNo + COM_OBJ_POS_VALID, 0);
#endif
}

void Channel::objectUpdateCh(unsigned int objno)
{
    unsigned int fct = objno - firstObjNo;
    unsigned int value = bcu.comObjects->objectRead(objno);
    Blind * blind = NULL;
    if (channelType() == BLIND)
        blind = (Blind *) this;
    switch (fct)
    {
    case COM_OBJ_UP_DOWN : // object for move Up/Down (long)
        if (! (activeLocks & LOCK_UP_DOWN))
            handleMove(value);
        break;
    case COM_OBJ_SLAT : // object for Stop, slat open/close (short)
        handleStep(value);
        break;
    case COM_OBJ_STOP: // object for stopping the movement
        stop();
        break;
    case COM_OBJ_SCENE: // object for scene
        if (! (activeLocks & LOCK_SCENE))
            handleScene(value);
        break;
    case COM_OBJ_SET_POSITION: // object for requested position
        if (! (activeLocks & LOCK_ABSOLUTE_POSITION))
            moveTo(value);
        break;
    case COM_OBJ_SET_SLAT_POSITION: // object for requested slat position
        if (! (activeLocks & LOCK_ABSOLUTE_POSITION))
            if (blind)
                blind->moveSlatTo(value);
        break;
    case COM_OBJ_START_REF_DRIVE: // object for start reference drive
        _savePosition (features & FEATURE_RESTORE_AFTER_REF);
        moveTo        (0);
        if (blind)
            blind->moveSlatTo    (0);
        break;
    case COM_OBJ_1_BIT_ACTION: // object for move to limit (1 bit)/move to position (1 bit)
        if (!obj24Config)
        {   // configured as drive to limit
            if (value)
                moveTo(topLimitPos);
            else
                moveTo(botLimitPos);
        }
        else
        {   // configured as drive to defined position
            if (value)
            {
                unsigned int cfg;
                cfg = obj24Config >> 5;
                if (   (cfg == 1)
                   || ((cfg == 3) && (position == 0))
                   || ((cfg == 5) && (position == 255))
                   )
                    _moveToOneBitPostion();
            }
            else
            {   // the config is the same as for the alarms -> we reuse the function
                _alarmAction(obj24Config >> 3);
            }
        }
        break;
    case COM_OBJ_LOCK_ABS_POSITION: // object for lock absolute position
        // TODO: implement lock absolute positioning
        break;
    case COM_OBJ_LOCK_UNIVERSAL: // object for lock universal
        if (value)
        {
            activeLocks = lockConfig;
        }
        else
            activeLocks = 0;
        break;
    case COM_OBJ_WIND_ALARM: // object for wind alert
        _checkAlarms(0, value);
        break;
    case COM_OBJ_RAIN_ALARM: // object for rain alert
        _checkAlarms(1, value);
       break;
    case COM_OBJ_FROST_ALARM: // object for frost alert
        _checkAlarms(2, value);
        break;
    case COM_OBJ_LOCK: // object for lock
        _checkAlarms(3, value);
        break;

        // the following objects are send only
    case COM_OBJ_VISU_STATUS: // object for state of movement
    case COM_OBJ_POSITION: // object for status current position
    case COM_OBJ_SLAT_POSITION: // object for status current slat position
    case COM_OBJ_POS_VALID: // object for position valid
    case COM_OBJ_IN_TOP_LIMIT: // object for status top position
    case COM_OBJ_IN_BOT_LIMIT: // object for status bottom position
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
        switchOutputPin(outputPins[number * 2 + 0], OUTPUT_LOW);
        switchOutputPin(outputPins[number * 2 + 1], OUTPUT_LOW);

        if (handAct_ != nullptr)
        {
            handAct_->setLedState(number * 2 + 0, 0);
            handAct_->setLedState(number * 2 + 1, 0);
        }

        state     = PROTECT;
        timeout.start(pauseChangeDir);
        _sendPosition();
        if (features & FEATURE_STATUS_MOVING)
            bcu.comObjects->objectWrite(firstObjNo + COM_OBJ_VISU_STATUS, (unsigned int) 0);
    }
}

void Channel::_sendPosition()
{
    bcu.comObjects->objectWrite(firstObjNo + COM_OBJ_POSITION, position);
}

/*
 * delays switching actions on the channel for ms milliseconds
 *
 */
bool Channel::delaySwitchingForMs(int ms)
{
    bool bDelaySwitching;
    bDelaySwitching = (state == IDLE) || (state == PROTECT);
    if (bDelaySwitching)
    {
        if (timeout.expired ())
        {
            timeout.start (ms);
            state = PROTECT;
        }
        else
        {
            //TODO need to know the remaining ms of the timeout, to decide weather or not we have to restart the timeout timer
            timeout.start(ms);
            state = PROTECT;
        }
    }
    return bDelaySwitching;
}

/*
 * use switchOutputPin which will call digitalWrite, so we can track then the output was set to high
 */
void Channel::switchOutputPin(int OutputPin, OutputState state)
{
    if (state == OUTPUT_HIGH)
    {
        //TODO save requested Outputpin and state, so we can later in UpdateRelayState() set it with digitalWrite
        digitalWrite(OutputPin,state);
        Blocking.start(BLOCKING_MS);
    }
    else
    {
        digitalWrite(OutputPin,state);
        Blocking.stop();
    }
}

/*
 * returns true, while the "cooldown" of an recently high switched OutputPin is blocking other channels from doing the same
 */
bool Channel::isBlocking()
{
    return Blocking.started();
}

bool Channel::UpdateRelayState()
{
    //TODO switch relay according to the requested state from switchOutputPin
    //digitalWrite(OutputPin,state);
    return false;
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
            if (!positionValid)
            {
                positionValid = true;
                bcu.comObjects->objectWrite(firstObjNo + COM_OBJ_POS_VALID, 1);
            }
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
            switchOutputPin(outputPins[outNo], OUTPUT_HIGH);
            if (features & FEATURE_STATUS_MOVING)
                bcu.comObjects->objectWrite(firstObjNo + COM_OBJ_VISU_STATUS, 1);
            else
                bcu.comObjects->objectWrite(firstObjNo + COM_OBJ_VISU_STATUS, (int) (direction == UP ? 0 : 1));

            Channel::setPWMtoMaxDuty();     // set PWM to maximum pulse width so relays can switch
            PWMDisabled.start(PWM_TIMEOUT); // start timer to reset PWM back to normal pulse width

            if (handAct_ != nullptr)
                handAct_->setLedState(outNo, 1);

            timeout.start(motorOnDelay);
            state = DELAY;
        }
        break;
    }
}

bool Channel::_trackPosition(void)
{
    // We assume that the movement has started motorOffDelay earlier to reach the current
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
    bcu.comObjects->objectSetValue(firstObjNo + COM_OBJ_POSITION, position);
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
    _updatePosState(position <= topLimitPos, IN_TOP_POSITION, COM_OBJ_IN_TOP_LIMIT);
    _updatePosState(position >= botLimitPos, IN_BOT_POSITION, COM_OBJ_IN_BOT_LIMIT);
    return false;
}

bool Channel::_trackSlatPosition(void)
{
    state = MOVE;
    return true;
}

void Channel::periodic(void)
{
    AlarmConfig * alarm = alarms;
    _handleState();
    for (unsigned int i = 0; i < NO_OF_ALARMS; i++, alarm++)
    {
        if (  (alarm->priority & activeAlarms)
           && alarm->monitorTime
           && alarm->monitor.expired()
           )
        {   // this alarm is active, should be monitored and the timeout as expired
            // we tread this alarm as inactive
            _checkAlarms(i, 0);
        }
    }
}

void Channel::_updatePosState(unsigned int current, unsigned int mask, unsigned int objno)
{
    objno += firstObjNo;
    if ((limits & mask) && !current)
    {
        limits &= ~mask;
        bcu.comObjects->objectWrite(objno, (unsigned int) 0);
    }
    if (!(limits & mask) && current)
    {
        limits |= mask;
        bcu.comObjects->objectWrite(objno, (unsigned int) 1);
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

void Channel::handleAutomaticFunction(unsigned int pos, unsigned int block, unsigned int value)
{
    if (  ! (activeLocks & LOCK_AUTOMATIC_MODE)
       && (  (!block && automaticAEnabled())
          || ( block && automaticBEnabled())
          )
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
    if (isRunning())
        stop();
    else
    {
        if (channelType() == BLIND)
        {
            Blind * blind = (Blind *) this;
            if (value)
            {
                blind->moveSlatFor(shortTime, DOWN);
            }
            else
            {
                blind->moveSlatFor(shortTime, UP);
            }
        }
        else if (shortTime)
        {
            if (value)
            {
                moveFor(shortTime, DOWN);
            }
            else
            {
                moveFor(shortTime, UP);
            }

        }
    }
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
                        bcu.userEeprom->modified(true);
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
    unsigned int address = currentVersion.baseAddress + EE_CHANNEL_CFG_SIZE * number;
    scenePos [i] = position;
    if ((*(bcu.userEeprom))[address + 32 + i] != position)
    {
        (*(bcu.userEeprom))[address + 32 + i] = position;
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

    if (!activeAlarms && (savedPosition != -1))
    {
        moveTo (savedPosition);
        savedPosition = -1;
        return true;
    }
    return false;
}

void Channel::_enableFeature(unsigned int address, unsigned int feature, unsigned int mask)
{
    if (bcu.userEeprom->getUInt8(address) & mask)
    {
        features |= feature;
    }
}

void Channel::_checkAlarms(unsigned int alarmNo, unsigned int value)
{
    AlarmConfig * alarm = & alarms[alarmNo];
    bool active = activeAlarms & alarm->priority;
    if (value && alarm->monitorTime)
    {   // this is sent as active and we have to monitor the timeout ->
        // re-trigger the timeout
        alarm->monitor.start(alarm->monitorTime * 1000 * 60);
    }
    if (!value && active)
    {   // the alarm was active and should now be disabled
        activeAlarms &= ~alarm->priority;
        if (!activeAlarms)
        {   // this was the last active alarm -> perform the alarm off action
            _alarmAction(reactionLockRemove);
        }
        if (activeAlarms < alarm->priority)
        {   // an active alarm with lower priority is now the highest active alarm
            // check the actions of that alarm
            for (unsigned int i = 0; i < NO_OF_ALARMS; i++)
            {
                alarm = & alarms[i];
                if (  (activeAlarms &  alarm->priority)
                   && ((activeAlarms & ~alarm->priority) < alarm->priority)
                   )
                {   // this is the active alarm with the highest priority
                    _alarmAction(alarm->engageAction);
                    break;
                }
            }
        }
    }
    else if (value && !active)
    {   // the alarm was inactive and now becomes active
        if (!activeAlarms)
        {   // this is the first active alarm -> save the position
            _savePosition(true);
        }
        if (activeAlarms < alarm->priority)
        {   // this alarm becomes now the alarm with the highest priority
            _alarmAction(alarm->engageAction);
        }
        activeAlarms |= alarm->priority;
    }
}

void Channel::_alarmAction(unsigned int action)
{
    switch (action)
    {
    case 1:
        moveTo(0); break;
    case 2:
        moveTo(255); break;
    case 4:
        _restorePosition(); break;
    }
}

void Channel::_moveToOneBitPostion()
{
    moveTo(oneBitPosition);
}

void Channel::setHandActuation(HandActuation* hand)
{
    handAct_ = hand;
}
