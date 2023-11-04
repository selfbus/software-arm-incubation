/*
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

/**
 * Den Alarm Status auf den Bus senden falls noch nicht gesendet.
 *
 * @param newAlarm - neuer Alarm Status
 *//*
void SmokeDetectorAlarm::send_obj_alarm(bool newAlarm)
{
    if (deviceHasAlarmLocal != newAlarm)
    {
        objectWrite(GroupObject::grpObjAlarmBus, newAlarm);
        if()
        objectWrite(GroupObject::grpObjStatusAlarm, newAlarm);
    }
}*/

/**
 * Den Testalarm Status auf den Bus senden falls noch nicht gesendet.
 *
 * @param newAlarm - neuer Testalarm Status
 */
void SmokeDetectorAlarm::send_obj_test_alarm(bool newAlarm)
{
    if (deviceHasTestAlarmLocal != newAlarm)
    {
        groupObjects->write(GroupObject::grpObjTestAlarmBus, newAlarm);
        groupObjects->write(GroupObject::grpObjStatusTestAlarm, newAlarm);
    }
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

        // Wenn wir lokalen Alarm haben dann Bus Alarm wieder auslösen
        // damit der Status der anderen Rauchmelder stimmt
        if (!requestedAlarmBus && deviceHasAlarmLocal)
            groupObjects->write(GroupObject::grpObjAlarmBus, deviceHasAlarmLocal);

        if (ignoreBusAlarm)
            requestedAlarmBus = false;
    }
    else if (groupObject == GroupObject::grpObjTestAlarmBus) // Test Alarm Network
    {
        requestedTestAlarmBus = groupObjects->read(groupObject) & 0x01;

        // Wenn wir lokalen Testalarm haben dann Bus Testalarm wieder auslösen
        // damit der Status der anderen Rauchmelder stimmt
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
    if (config->alarmSendDelayed() && newAlarmLocal) // wenn Alarm verzögert gesendet werden soll und Alarm ansteht
    {
        delayedAlarmCounter = config->alarmDelaySeconds();
        groupObjects->setValue(GroupObject::grpObjStatusAlarmDelayed, true);
    }
    else if (deviceHasAlarmLocal != newAlarmLocal) //wenn Alarm nicht verzögert gesendet werden soll oder Alarm nicht mehr ansteht (nur 1x senden)
    {
        groupObjects->write(GroupObject::grpObjAlarmBus, newAlarmLocal);
    }

    if (deviceHasAlarmLocal != newAlarmLocal) //sobald neuer AlarmStatus ansteht, soll dieser versendet werden
    {
        groupObjects->write(GroupObject::grpObjStatusAlarm, newAlarmLocal);
    }

    deviceHasAlarmLocal = newAlarmLocal;

    send_obj_test_alarm(newTestAlarmLocal);
    deviceHasTestAlarmLocal = newTestAlarmLocal;

    // Bus Alarm
    deviceHasAlarmBus = newAlarmFromBus;

    // Bus Testalarm
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
     * Somit wird die Status Nachricht EIN 2x versendet (1x aus send_obj_alarm bzw. send_obj_test_alarm) und einmal hier.
     * AUS wird hier allerdings nicht versendet, da requestedAlarmBus bzw. requestedTestAlarmBus dann false sind
     *
     * Daher habe ich mich entschieden, diese Versendung vorerst zu deaktivieren
     */

    if (requestedAlarmBus) //wenn Alarm auf Bus anliegt
    {
        requestedAlarmBus = false;
        delayedAlarmCounter = 0; // verzögerten Alarm abbrechen
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
            --delayedAlarmCounter;
            if (!delayedAlarmCounter)   // Verzögerungszeit abgelaufen
            {
                groupObjects->setValue(GroupObject::grpObjStatusAlarmDelayed, false); // Status verzögerter Alarm zurücksetzen
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
