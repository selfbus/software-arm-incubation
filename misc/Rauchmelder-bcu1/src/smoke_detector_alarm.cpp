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

    alarmCounter = 1;
    testAlarmCounter = 1;
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
            groupObjects->write(GroupObject::grpObjAlarmBus, deviceHasAlarmLocal);

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
            groupObjects->write(GroupObject::grpObjTestAlarmBus, deviceHasTestAlarmLocal);

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
            groupObjects->write(GroupObject::grpObjAlarmBus, newAlarmLocal);
        }

        // Send out the new status immediately.
        groupObjects->write(GroupObject::grpObjStatusAlarm, newAlarmLocal);
        deviceHasAlarmLocal = newAlarmLocal;
    }

    // Local Test Alarm: Send out TestAlarmStatus and TestAlarmNetwork.
    if (deviceHasTestAlarmLocal != newTestAlarmLocal)
    {
        groupObjects->write(GroupObject::grpObjTestAlarmBus, newTestAlarmLocal);
        groupObjects->write(GroupObject::grpObjStatusTestAlarm, newTestAlarmLocal);
        deviceHasTestAlarmLocal = newTestAlarmLocal;
    }

    // Bus Alarm and Bus Test Alarm: Just remember current status for correct behavior in
    // loopCheckAlarmState().
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
        //objectWrite(GroupObject::grpObjStatusAlarm, deviceHasAlarmLocal);
    }

    if (requestedTestAlarmBus) //wenn Testalarm auf Bus anliegt
    {
        requestedTestAlarmBus = false;
        //objectWrite(GroupObject::grpObjStatusTestAlarm, deviceHasTestAlarmLocal);
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
            return RM_ALARM;
    }
    else if (requestedTestAlarmBus)
    {
        // Trigger Test Alarm if necessary, or fall through to No Change.
        if (!deviceHasTestAlarmBus)
            return RM_TEST_ALARM;
    }
    else if (deviceHasAlarmBus || deviceHasTestAlarmBus)
    {
        // Stop Alarm and Test Alarm.
        return RM_NO_ALARM;
    }

    return RM_NO_CHANGE;
}

void SmokeDetectorAlarm::timerEverySecond()
{
    // Alarm: verzögert und zyklisch senden
    if (deviceHasAlarmLocal)
    {
        // Alarm verzögert senden
        if (delayedAlarmCounter)
        {
            setDelayedAlarmCounter(delayedAlarmCounter - 1);
            if (!delayedAlarmCounter)   // Verzögerungszeit abgelaufen
            {
                //groupObjects->send(GroupObject::grpObjAlarmBus);  // Vernetzung Alarm senden
                //groupObjects->send(GroupObject::grpObjStatusAlarm); // Status Alarm senden

                groupObjects->write(GroupObject::grpObjAlarmBus, deviceHasAlarmLocal);
            }
        }
        else // Alarm zyklisch senden
        {
            if (config->alarmSendStatusPeriodically())
            {
                --alarmCounter;
                if (!alarmCounter)
                {
                    alarmCounter = config->alarmIntervalSeconds();     // Zykl. senden Zeit holen
                    if (config->alarmSendNetworkPeriodically())
                    {
                        groupObjects->send(GroupObject::grpObjAlarmBus); // Vernetzung Alarm senden
                    }
                    groupObjects->send(GroupObject::grpObjStatusAlarm);
                }
            }
        }
    }
    // Kein Alarm, zyklisch 0 senden
    else
    {
        if (config->alarmSendStatusPeriodicallyWhenNoAlarm())
        {
            --alarmCounter;
            if (!alarmCounter)
            {
                alarmCounter = config->alarmIntervalSeconds(); // Zykl. senden Zeit holen
                groupObjects->send(GroupObject::grpObjStatusAlarm);
            }
        }
    }

    // Testalarm: zyklisch senden
    if (deviceHasTestAlarmLocal)
    {
        if (config->testAlarmSendStatusPeriodically())
        {
            --testAlarmCounter;
            if (!testAlarmCounter)
            {
                testAlarmCounter = config->testAlarmIntervalSeconds();
                if (config->testAlarmSendNetworkPeriodically())
                {
                    groupObjects->send(GroupObject::grpObjTestAlarmBus);
                }
                groupObjects->send(GroupObject::grpObjStatusTestAlarm);
            }
        }
    }
}

void SmokeDetectorAlarm::timerEveryMinute()
{
    // Bus Alarm ignorieren Flag rücksetzen wenn kein Alarm mehr anliegt
    if (ignoreBusAlarm & !(deviceHasAlarmBus | deviceHasTestAlarmBus))
    {
        ignoreBusAlarm = false;
        groupObjects->setValue(GroupObject::grpObjResetAlarm, ignoreBusAlarm);
    }
}

void SmokeDetectorAlarm::setDelayedAlarmCounter(uint8_t newValue)
{
    delayedAlarmCounter = newValue;
    groupObjects->writeIfChanged(GroupObject::grpObjStatusAlarmDelayed, delayedAlarmCounter != 0);
}
