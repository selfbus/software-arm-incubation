/*
 *    _____ ________    __________  __  _______    ____  __  ___
 *   / ___// ____/ /   / ____/ __ )/ / / / ___/   / __ \/  |/  /
 *   \__ \/ __/ / /   / /_  / __  / / / /\__ \   / /_/ / /|_/ /
 *  ___/ / /___/ /___/ __/ / /_/ / /_/ /___/ /  / _, _/ /  / /
 * /____/_____/_____/_/   /_____/\____//____/  /_/ |_/_/  /_/
 *
 *  Original written for LPC922:
 *  Copyright (c) 2016 Stefan Haller
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  Modified for LPC1115 ARM processor:
 *  Copyright (c) 2017-2022 Oliver Stefan <o.stefan252@googlemail.com>
 *
 *  Refactoring and bug fixes:
 *  Copyright (c) 2023 Darthyson <darth@maptrack.de>
 *  Copyright (c) 2023 Thomas Dallmair <dev@thomas-dallmair.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <stdint.h>
#include <type_traits>

#include "smoke_detector_app.h"
#include "rm_const.h"
#include "smoke_detector_alarm.h"
#include "smoke_detector_config.h"
#include "smoke_detector_device.h"
#include "smoke_detector_errorcode.h"
#include "smoke_detector_group_objects.h"

extern SmokeDetectorApp *app;

SmokeDetectorApp::SmokeDetectorApp()
    : bcu(BCU1()),
      config(new SmokeDetectorConfig(bcu.userEeprom)),
      groupObjects(new SmokeDetectorGroupObjects(bcu.comObjects)),
      alarm(new SmokeDetectorAlarm(config, groupObjects)),
      errorCode(new SmokeDetectorErrorCode(groupObjects)),
      device(new SmokeDetectorDevice(config, groupObjects, alarm, errorCode))
{
    infoSendObjno = GroupObject::grpObjAlarmBus;
    readCmdno = device->commandTableSize();
    infoCounter = config->infoIntervalMinutes();
    eventTime = DefaultEventTime;
}

BcuBase* SmokeDetectorApp::initialize()
{
    // Manufacturer code 0x004C = Robert Bosch, Device type 1010 (0x03F2), Version 2.4
    bcu.begin(0x004C, 0x03F2, 0x24);
    setupPeriodicTimer(TimerIntervalMs);
    return (&bcu);
}

void SmokeDetectorApp::loop()
{
    device->recv_bytes();
    updateAlarmState();
    handleUpdatedGroupObjects();

    // Sleep up to 1 millisecond if there is nothing to do
    if (bcu.bus->idle())
        waitForInterrupt();
}

void SmokeDetectorApp::loopNoApp()
{
    device->recv_bytes(); // timer32_0 is still running, so we should read the received bytes
}

void SmokeDetectorApp::setupPeriodicTimer(uint32_t milliseconds)
{
    // Enable the timer interrupt
    enableInterrupt(TIMER_32_0_IRQn);

    // Begin using the timer
    timer32_0.begin();

    // Smoke detector timer is important, but less important than the Bus timer.
    timer32_0.setIRQPriority(1);

    // Let the timer count milliseconds
    timer32_0.prescaler((SystemCoreClock / 1000) - 1);

    // On match of MAT1, generate an interrupt and reset the timer
    timer32_0.matchMode(MAT1, RESET | INTERRUPT);

    // Match MAT1 when the timer reaches this value (in milliseconds)
    timer32_0.match(MAT1, milliseconds - 1); // -1 because counting starts from 0, e.g. 0-499=500ms

    timer32_0.start();
}

/**
 * Continuously check device alarm state. When we think the bus alarm should have a certain
 * state, then tell the smoke detector device until it agrees.
 */
void SmokeDetectorApp::updateAlarmState()
{
    if (device->hasOngoingMessageExchange()) ///\todo while waiting a answer we don't process alarms? rly?
    {
       return;
    }

    auto alarmState = alarm->loopCheckAlarmState();
    if (alarmState != RmAlarmState::noChange)
    {
        device->set_alarm_state(alarmState);
    }
}

/**
 * Handle updates to group objects via KNX bus
 */
void SmokeDetectorApp::handleUpdatedGroupObjects()
{
    int objno;

    while ((objno = bcu.comObjects->nextUpdatedObject()) >= 0)
    {
        auto groupObject = static_cast<GroupObject>(objno);
        if (groupObject == GroupObject::grpObjAlarmBus ||         // Alarm Network
            groupObject == GroupObject::grpObjTestAlarmBus ||     // Test Alarm Network
            groupObject == GroupObject::grpObjResetAlarm)         // Alarm Reset
        {
            alarm->groupObjectUpdated(groupObject);
        }
    }
}

/**
 * Timer Event.
 * Is called every 500ms
 */
extern "C" void TIMER32_0_IRQHandler()
{
    // Clear the timer interrupt flags. Otherwise the interrupt handler is called
    // again immediately after returning.
    timer32_0.resetFlags();

    app->timer();
}

void SmokeDetectorApp::timer()
{
    --eventTime;

    device->timerEvery500ms();

    // Everything else just needs to be done every second
    if (eventTime & 1)
        return;

    alarm->timerEverySecond();
    auto hasAlarm = alarm->hasAlarm();

    // Send an informational group object every other second if there is no alarm.
    if ((eventTime % DefaultKnxObjectTime) == 0 && infoSendObjno && !hasAlarm)
    {
        // Mark the informational group object for sending if it is configured as such.
        if (config->infoSendPeriodically(infoSendObjno))
        {
            groupObjects->send(infoSendObjno);
        }

        infoSendObjno = static_cast<GroupObject>(static_cast<std::underlying_type_t<GroupObject>>(infoSendObjno) - 1);
    }

    // Send one of the smoke detector commands every 8 seconds in order to retrieve all status data.
    if ((eventTime % DefaultSerialCommandTime) == 0 && readCmdno && !hasAlarm)
    {
        if (!device->hasOngoingMessageExchange())
        {
            readCmdno--;
            if (!device->send_Cmd((Command)readCmdno))
            {
                readCmdno++;
            }
        }
    }

    // Once per minute
    if (!eventTime)
    {
        eventTime = DefaultEventTime;

        alarm->timerEveryMinute();

        if (!readCmdno)
        {
            readCmdno = device->commandTableSize();
        }

        // Send status information periodically
        if (config->infoSendAnyPeriodically())
        {
            --infoCounter;
            if (!infoCounter)
            {
                infoCounter = config->infoIntervalMinutes();
                infoSendObjno = OBJ_HIGH_INFO_SEND;
            }
        }
    }
}
