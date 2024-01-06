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
#include <sblib/bits.h>

#include "sd_app.h"
#include "sd_alarm.h"
#include "sd_config.h"
#include "sd_device.h"
#include "sd_errorcode.h"
#include "sd_group_objects.h"
#include "sd_shared_enums.h"

extern SmokeDetectorApp *app;
extern "C" void TIMER32_0_IRQHandler();

SmokeDetectorApp::SmokeDetectorApp()
    : bcu(BCU1()),
      config(new SmokeDetectorConfig(bcu.userEeprom)),
      groupObjects(new SmokeDetectorGroupObjects(bcu.comObjects)),
      alarm(new SmokeDetectorAlarm(config, groupObjects)),
      errorCode(new SmokeDetectorErrorCode(groupObjects)),
      device(new SmokeDetectorDevice(config, groupObjects, alarm, errorCode))
{
    isTimerInitialized = false;
    burstPreventionDelay.stop();
    startSendingInfoGroupObjects();
    deviceCommand = AllDeviceCommands().begin();
    eventTime = DefaultEventTime;
}

BcuBase* SmokeDetectorApp::initialize()
{
    // Manufacturer code 0x004C = Robert Bosch, Device type 1010 (0x03F2), Version 2.4
    bcu.begin(0x004C, 0x03F2, 0x24);

    // Prevent message bursts caused by periodic sending of info group objects. If there are 10 devices
    // on the bus and bus power gets reset for whatever reason, all of these devices would try to send
    // their info group objects at the same time (provided they all have the same configuration, but
    // that seems likely).
    // Prevent this by artificially delaying startup by a pseudo-random delay (actually specific to
    // the individual address).
    auto randomByte = getRandomUInt8();

    // Delay for randomByte * 8ms. Maximum delay is then 250 * 8ms = 2000ms. That's great because
    // we're sending info group objects every 2 seconds, so the sending of different communication
    // objects does almost certainly not overlap.
    // Also, a 4-byte group write message takes ~21.4ms on TP1 (if I got the math right), so all
    // devices where randomByte differs by at least 3 should not interfere with each other. They
    // even leave a time window of a few milliseconds for other unrelated devices to sneak in their
    // frames, causing very little delay for them.
    burstPreventionDelay.start(randomByte << 3);

    return (&bcu);
}

void SmokeDetectorApp::loop()
{
    // Do nothing while waiting for the burstPreventionDelay to expire. Need to call both
    // stopped() and expired(), because expired() is the only method that actually checks
    // whether the timeout expired or not, then stops it.
    if (burstPreventionDelay.stopped() || burstPreventionDelay.expired())
    {
        device->loopCheckState();
        device->loopReceiveBytes();
        updateAlarmState();
        handleUpdatedGroupObjects();

        // Only after establishing a connection to the device, we're able to read data from
        // it periodically and then also send out the informational group objects. Thus, only
        // start the timer after the device becomes ready.
        if (!isTimerInitialized && device->isReady())
        {
            setupPeriodicTimer(TimerIntervalMs);
            isTimerInitialized = true;
        }
    }

    // Sleep up to 1 millisecond if there is nothing to do
    if (bcu.bus->idle())
        waitForInterrupt();
}

void SmokeDetectorApp::loopNoApp()
{
    device->loopReceiveBytes(); // timer32_0 is still running, so we should read the received bytes
}

/**
 * Returns a pseudo-random number in the range 0 to 250, inclusive.
 */
uint8_t SmokeDetectorApp::getRandomUInt8()
{
    // For example, 1.2.15 is 0x120F. It's highly likely that multiple smoke detectors are in
    // the same area and line, and even have consecutive device numbers. Thus, reverse the bytes
    // before multiplying with some prime and calculating modulo the biggest prime below 256,
    // to spread the results more evenly.
    //
    // This algorithm yields the following results with primes 29 and 251:
    //     1.1.1 -> 136
    //     1.1.2 -> 30
    //     1.1.3 -> 175
    //     1.1.4 -> 69
    //     1.1.5 -> 214
    //     1.1.6 -> 108
    //     1.1.7 -> 2
    //     1.1.8 -> 147
    //     1.1.9 -> 41
    //     1.1.10 -> 186
    // Clustering starts with the 46th device with value 135.

    auto ownAddress = bcu.ownAddress();
    auto reversedBytes = makeWord(lowByte(ownAddress), highByte(ownAddress));
    auto randomNumber = (reversedBytes * 29) % 251;
    return static_cast<uint8_t>(randomNumber);
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
    timer32_0.match(MAT1, milliseconds - 1); // -1 because counting starts from 0, e.g. 0-999=1000ms

    timer32_0.start();
}

/**
 * Continuously check device alarm state. When we think the bus alarm should have a certain
 * state, then tell the smoke detector device until it agrees.
 */
void SmokeDetectorApp::updateAlarmState()
{
    auto alarmState = alarm->loopCheckAlarmState();
    if (alarmState != RmAlarmState::noChange)
    {
        device->setAlarmState(alarmState);
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
        if (groupObject == GroupObject::alarmBus ||         // Alarm Network
            groupObject == GroupObject::testAlarmBus ||     // Test Alarm Network
            groupObject == GroupObject::resetAlarm)         // Alarm Reset
        {
            alarm->groupObjectUpdated(groupObject);
        }
    }
}

/**
 * Send informational group objects in this newly started minute.
 */
void SmokeDetectorApp::startSendingInfoGroupObjects()
{
    infoCounter = config->infoIntervalMinutes();
    infoGroupObject = InfoGroupObjects().begin();
}

/**
 * Timer Event.
 * Is called every 1000ms
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

    alarm->timerEverySecond();
    auto hasAlarm = alarm->hasAlarm();

    // Send an informational group object every other second if there is no alarm. Only start
    // sending these after we have sent at least 3 requests to the device, which corresponds
    // to DeviceCommand::smokeboxData. This ensures that each group object is filled with
    // correct and up-to-date values before we send it onto the bus.
    if ((eventTime % DefaultKnxObjectTime) == 0 && infoGroupObject != infoGroupObject.end() && !hasAlarm &&
        (*deviceCommand) > DeviceCommand::smokeboxData)
    {
        // Mark the informational group object for sending if it is configured as such.
        if (config->infoSendPeriodically(*infoGroupObject))
        {
            groupObjects->send(*infoGroupObject);
        }

        ++infoGroupObject;
    }

    // Send one of the smoke detector commands every 8 seconds in order to retrieve all status data.
    if ((eventTime % DefaultSerialCommandTime) == 0 && deviceCommand != deviceCommand.end() && !hasAlarm)
    {
        if (device->sendCommand(*deviceCommand))
        {
            ++deviceCommand;
        }
    }

    // Once per minute
    if (!eventTime)
    {
        eventTime = DefaultEventTime;

        alarm->timerEveryMinute();

        if (deviceCommand == deviceCommand.end())
        {
            deviceCommand = AllDeviceCommands().begin();
        }

        // Send status information periodically
        if (config->infoSendAnyPeriodically())
        {
            --infoCounter;
            if (!infoCounter)
            {
                startSendingInfoGroupObjects();
            }
        }
    }
}
