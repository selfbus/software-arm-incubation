/*  Original written for LPC922:
 *  Copyright (c) 2015-2017 Stefan Haller
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  Modified for LPC1115 ARM processor:
 *  Copyright (c) 2017 Oliver Stefan <o.stefan252@googlemail.com>
 *  Copyright (c) 2020 Stefan Haller
 *
 *  Refactoring and bug fixes:
 *  Copyright (c) 2023 Darthyson <darth@maptrack.de>
 *  Copyright (c) 2023 Thomas Dallmair <dev@thomas-dallmair.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include "smoke_detector_alarm.h"
#include "smoke_detector_config.h"
#include "smoke_detector_group_objects.h"

SmokeDetectorAlarm::SmokeDetectorAlarm(const SmokeDetectorConfig *config, const SmokeDetectorGroupObjects *groupObjects)
    : config(config),
      groupObjects(groupObjects)
{
    deviceHasAlarmLocal = false;
    deviceHasAlarmBus = false;

    deviceHasTestAlarmLocal = false;
    deviceHasTestAlarmBus = false;

    requestedAlarmBus = false;
    requestedTestAlarmBus = false;
    ignoreBusAlarm = false;

    alarmNetworkCounter = config->alarmIntervalSeconds();
    alarmStatusCounter = config->alarmIntervalSeconds();
    testAlarmCounter = config->testAlarmIntervalSeconds();
    delayedAlarmCounter = 0;
}

bool SmokeDetectorAlarm::hasAlarm() const
{
    return deviceHasAlarmLocal | deviceHasAlarmBus | deviceHasTestAlarmLocal | deviceHasTestAlarmBus;
}

void SmokeDetectorAlarm::groupObjectUpdated(GroupObject groupObject)
{
    if (groupObject == GroupObject::grpObjAlarmBus) // Alarm Network
    {
        requestedAlarmBus = groupObjects->read(groupObject) & 0x01;

        // If anybody requested Alarm Network to be cleared, but we do have an ongoing local
        // alarm, then trigger the Alarm Network again. This ensures all connected smoke
        // detectors are in consistent state. Only exception is a delayed alarm that is still
        // counting down.
        if (!requestedAlarmBus && deviceHasAlarmLocal && !delayedAlarmCounter)
            sendAlarmNetwork();

        if (ignoreBusAlarm)
            requestedAlarmBus = false;
    }
    else if (groupObject == GroupObject::grpObjTestAlarmBus) // Test Alarm Network
    {
        requestedTestAlarmBus = groupObjects->read(groupObject) & 0x01;

        // If anybody requested Test Alarm Network to be cleared, but we do have an ongoing local
        // test alarm, then trigger the Test Alarm Network again. This ensures all connected smoke
        // detectors are in consistent state.
        if (!requestedTestAlarmBus && deviceHasTestAlarmLocal)
            sendTestAlarmNetwork();

        if (ignoreBusAlarm)
            requestedTestAlarmBus = false;
    }
    else if (groupObject == GroupObject::grpObjResetAlarm) // Alarm Reset
    {
        requestedAlarmBus = false;
        requestedTestAlarmBus = false;
        ignoreBusAlarm = true;
    }
}

void SmokeDetectorAlarm::deviceStatusUpdate(bool newAlarmLocal, bool newTestAlarmLocal, bool newAlarmFromBus, bool newTestAlarmFromBus)
{
    // Local Alarm: Send out AlarmStatus and either AlarmNetwork or AlarmDelayed, depending on
    // config, and set the delay timer correctly.
    if (deviceHasAlarmLocal != newAlarmLocal)
    {
        deviceHasAlarmLocal = newAlarmLocal;

        // When there is a new local alarm that should only be forwarded with some delay,
        // start the timer.
        if (newAlarmLocal && config->alarmSendDelayed())
        {
            setDelayedAlarmCounter(config->alarmDelaySeconds());
        }
        else
        {
            // Either a new local alarm that should be forwarded immediately, or a local
            // alarm ended. Both cases go to the bus right now. If the alarm started recently
            // and was not forwarded yet (only timer started), it's time to stop the timer.
            setDelayedAlarmCounter(0);
            sendAlarmNetwork();
        }

        // Send out the new status immediately.
        sendAlarmStatus();
    }

    // Local Test Alarm: Send out TestAlarmStatus and TestAlarmNetwork.
    if (deviceHasTestAlarmLocal != newTestAlarmLocal)
    {
        deviceHasTestAlarmLocal = newTestAlarmLocal;
        sendTestAlarmNetwork();
        sendTestAlarmStatus();
    }

    // Bus Alarm and Bus Test Alarm: Just remember current device status so we request to send
    // the correct alarm state in loopCheckAlarmState().
    deviceHasAlarmBus = newAlarmFromBus;
    deviceHasTestAlarmBus = newTestAlarmFromBus;
}

void SmokeDetectorAlarm::deviceButtonPressed()
{
    ///\todo see below
    /*
     * In der folgenden Passage ist für mich die Versendung der Objekte nicht nachvollziehbar:
     * Es wird kontrolliert, ob die Taste am Rauchmelder gedrückt wurde, anschließend wir überprüft, ob ein Alarm oder TestAlarm vom Bus vorliegt
     * Dann wird der jeweilige Status versendet.
     * für welchen Anwendungfall ist dieses sinnvoll?
     * zur Zeit wird vom lokalen Rauchmelder der requestedAlarmBus ausgelöst (quasi local loopback) und die Tastenerkennung löst aus
     * Somit wird die Status Nachricht EIN 2x versendet (1x aus deviceStatusUpdate) und einmal hier.
     * AUS wird hier allerdings nicht versendet, da requestedAlarmBus bzw. requestedTestAlarmBus dann false sind
     *
     * Daher habe ich mich entschieden, diese Versendung vorerst zu deaktivieren
     */

    if (requestedAlarmBus) //wenn Alarm auf Bus anliegt
    {
        requestedAlarmBus = false;
        setDelayedAlarmCounter(0); // verzögerten Alarm abbrechen
        //sendAlarmStatus();
    }

    if (requestedTestAlarmBus) //wenn Testalarm auf Bus anliegt
    {
        requestedTestAlarmBus = false;
        //sendTestAlarmStatus();
    }
}

/**
 * Continuously check device alarm state. When we think the bus alarm should have a certain
 * state, then tell the smoke detector device until it agrees.
 */
RmAlarmState SmokeDetectorAlarm::loopCheckAlarmState()
{
    if (requestedAlarmBus)
    {
        // Trigger Alarm if necessary, or fall through to No Change.
        if (!deviceHasAlarmBus)
            return RmAlarmState::RM_ALARM;
    }
    else if (requestedTestAlarmBus)
    {
        // Trigger Test Alarm if necessary, or fall through to No Change.
        if (!deviceHasTestAlarmBus)
            return RmAlarmState::RM_TEST_ALARM;
    }
    else if (deviceHasAlarmBus || deviceHasTestAlarmBus)
    {
        // Stop Alarm and Test Alarm.
        return RmAlarmState::RM_NO_ALARM;
    }

    return RmAlarmState::RM_NO_CHANGE;
}

void SmokeDetectorAlarm::timerEverySecond()
{
    // Alarm: Delayed and periodic sending
    if (deviceHasAlarmLocal)
    {
        // Delayed Alarm
        if (delayedAlarmCounter)
        {
            setDelayedAlarmCounter(delayedAlarmCounter - 1);
            if (!delayedAlarmCounter)
            {
                // Delay has expired, time to forward the alarm onto the bus.
                // AlarmStatus was already sent in deviceStatusUpdate(), so the only thing
                // to do here is to send AlarmNetwork.

                sendAlarmNetwork();
            }
        }
        else
        {
            // Periodic sending. This is in the else branch as it only starts after the delayed
            // alarm (if any) has expired.
            if (config->alarmSendStatusPeriodically())
            {
                --alarmStatusCounter;
                if (!alarmStatusCounter)
                {
                    // Send out AlarmStatus and restart the timer.
                    sendAlarmStatus();
                }

                if (config->alarmSendNetworkPeriodically())
                {
                    --alarmNetworkCounter;
                    if (!alarmNetworkCounter)
                    {
                        // Send out AlarmNetwork and restart the timer.
                        sendAlarmNetwork();
                    }
                }
            }
        }
    }
    // No Alarm: Periodic sending of status 0
    else
    {
        if (config->alarmSendStatusPeriodicallyWhenNoAlarm())
        {
            --alarmStatusCounter;
            if (!alarmStatusCounter)
            {
                sendAlarmStatus();
            }
        }
    }

    // Test Alarm: Periodic sending
    if (deviceHasTestAlarmLocal)
    {
        if (config->testAlarmSendStatusPeriodically())
        {
            --testAlarmCounter;
            if (!testAlarmCounter)
            {
                if (config->testAlarmSendNetworkPeriodically())
                {
                    sendTestAlarmNetwork();
                }
                sendTestAlarmStatus();
            }
        }
    }
}

void SmokeDetectorAlarm::timerEveryMinute()
{
    // Reset ignoreBusAlarm after bus alarm has been cleared.
    if (ignoreBusAlarm & !(deviceHasAlarmBus | deviceHasTestAlarmBus))
    {
        ignoreBusAlarm = false;
        groupObjects->setValue(GroupObject::grpObjResetAlarm, ignoreBusAlarm);
    }
}

void SmokeDetectorAlarm::sendAlarmNetwork()
{
    // Restart the timer for the next periodic sending interval.
    alarmNetworkCounter = config->alarmIntervalSeconds();
    groupObjects->write(GroupObject::grpObjAlarmBus, deviceHasAlarmLocal);
}

void SmokeDetectorAlarm::sendAlarmStatus()
{
    // Restart the timer for the next periodic sending interval.
    alarmStatusCounter = config->alarmIntervalSeconds();
    groupObjects->write(GroupObject::grpObjStatusAlarm, deviceHasAlarmLocal);
}

void SmokeDetectorAlarm::sendTestAlarmNetwork()
{
    // No dedicated timer for TestAlarmNetwork.
    groupObjects->write(GroupObject::grpObjTestAlarmBus, deviceHasTestAlarmLocal);
}

void SmokeDetectorAlarm::sendTestAlarmStatus()
{
    // Restart the timer for the next periodic sending interval.
    testAlarmCounter = config->testAlarmIntervalSeconds();
    groupObjects->write(GroupObject::grpObjStatusTestAlarm, deviceHasTestAlarmLocal);
}

void SmokeDetectorAlarm::setDelayedAlarmCounter(uint8_t newValue)
{
    delayedAlarmCounter = newValue;
    groupObjects->writeIfChanged(GroupObject::grpObjStatusAlarmDelayed, delayedAlarmCounter != 0);
}
