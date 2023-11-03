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
    alarmBus = 0;
    alarmLocal = 0;

    testAlarmBus = 0;
    testAlarmLocal = 0;

    setAlarmBus = 0;
    setTestAlarmBus = 0;
    ignoreBusAlarm = 0;

    alarmCounter = 1;
    TalarmCounter = 1;
    delayedAlarmCounter = 0;
}

/**
 * Den Alarm Status auf den Bus senden falls noch nicht gesendet.
 *
 * @param newAlarm - neuer Alarm Status
 *//*
void SmokeDetectorAlarm::send_obj_alarm(bool newAlarm)
{
    if (alarmLocal != newAlarm)
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
    if (testAlarmLocal != newAlarm)
    {
        groupObjects->write(GroupObject::grpObjTestAlarmBus, newAlarm);
        groupObjects->write(GroupObject::grpObjStatusTestAlarm, newAlarm);
    }
}

bool SmokeDetectorAlarm::hasAlarm() const
{
    return alarmLocal | alarmBus | testAlarmLocal | testAlarmBus;
}

void SmokeDetectorAlarm::groupObjectUpdated(GroupObject groupObject)
{
    if (groupObject == GroupObject::grpObjAlarmBus) // Bus Alarm
    {
        setAlarmBus = groupObjects->read(groupObject) & 0x01;

        // Wenn wir lokalen Alarm haben dann Bus Alarm wieder auslösen
        // damit der Status der anderen Rauchmelder stimmt
        if (!setAlarmBus && alarmLocal)
            groupObjects->write(GroupObject::grpObjAlarmBus, alarmLocal);

        if (ignoreBusAlarm)
            setAlarmBus = 0;
    }
    else if (groupObject == GroupObject::grpObjTestAlarmBus) // Bus Test Alarm
    {
        setTestAlarmBus = groupObjects->read(groupObject) & 0x01;

        // Wenn wir lokalen Testalarm haben dann Bus Testalarm wieder auslösen
        // damit der Status der anderen Rauchmelder stimmt
        if (!setTestAlarmBus && testAlarmLocal)
            groupObjects->write(GroupObject::grpObjTestAlarmBus, testAlarmLocal);

        if (ignoreBusAlarm)
            setTestAlarmBus = 0;
    }
    else if (groupObject == GroupObject::grpObjResetAlarm) // Bus Alarm rücksetzen
    {
        setAlarmBus = 0;
        setTestAlarmBus = 0;
        ignoreBusAlarm = 1;
    }
}

void SmokeDetectorAlarm::deviceStatusUpdate(bool newAlarmLocal, bool newTestAlarmLocal, bool newAlarmFromBus, bool newTestAlarmFromBus)
{
    if (config->alarmSendDelayed() && newAlarmLocal) // wenn Alarm verzögert gesendet werden soll und Alarm ansteht
    {
        delayedAlarmCounter = config->alarmDelaySeconds();
        groupObjects->setValue(GroupObject::grpObjStatusAlarmDelayed, true);
    }
    else if (alarmLocal != newAlarmLocal) //wenn Alarm nicht verzögert gesendet werden soll oder Alarm nicht mehr ansteht (nur 1x senden)
    {
        groupObjects->write(GroupObject::grpObjAlarmBus, newAlarmLocal);
    }

    if (alarmLocal != newAlarmLocal) //sobald neuer AlarmStatus ansteht, soll dieser versendet werden
    {
        groupObjects->write(GroupObject::grpObjStatusAlarm, newAlarmLocal);
    }

    alarmLocal = newAlarmLocal;

    send_obj_test_alarm(newTestAlarmLocal);
    testAlarmLocal = newTestAlarmLocal;

    // Bus Alarm
    alarmBus = newAlarmFromBus;

    // Bus Testalarm
    testAlarmBus = newTestAlarmFromBus;
}

void SmokeDetectorAlarm::deviceButtonPressed()
{
    ///\todo see below
    /*
     * In der folgenden Passage ist für mich die Versendung der Objekte nicht nachvollziehbar:
     * Es wird kontrolliert, ob die Taste am Rauchmelder gedrückt wurde, anschließend wir überprüft, ob ein Alarm oder TestAlarm vom Bus vorliegt
     * Dann wird der jeweilige Status versendet.
     * für welchen Anwendungfall ist dieses sinnvoll?
     * zur Zeit wird vom lokalen Rauchmelder der setAlarmBus ausgelöst (quasi local loopback) und die Tastenerkennung löst aus
     * Somit wird die Status Nachricht EIN 2x versendet (1x aus send_obj_alarm bzw. send_obj_test_alarm) und einmal hier.
     * AUS wird hier allerdings nicht versendet, da setAlarmBus bzw. setTestAlarmBus dann false sind
     *
     * Daher habe ich mich entschieden, diese Versendung vorerst zu deaktivieren
     */

    if (setAlarmBus) //wenn Alarm auf Bus anliegt
    {
        setAlarmBus = 0;
        delayedAlarmCounter = 0; // verzögerten Alarm abbrechen
        //objectWrite(GroupObject::grpObjStatusAlarm, alarmLocal);
    }

    if (setTestAlarmBus) //wenn Testalarm auf Bus anliegt
    {
        setTestAlarmBus = 0;
        //objectWrite(GroupObject::grpObjStatusTestAlarm, testAlarmLocal);
    }
}

/**
 * Den Zustand der Alarme bearbeiten. Wenn wir der Meinung sind der Bus-Alarm soll einen
 * bestimmten Zustand haben dann wird das dem Rauchmelder so lange gesagt bis der auch
 * der gleichen Meinung ist.
 */
RmAlarmState SmokeDetectorAlarm::process_alarm_stats()
{
    if (setAlarmBus && !alarmBus)
    {
        // Alarm auslösen
        return RM_ALARM;
    }

    if (setTestAlarmBus && !testAlarmBus)
    {
        // Testalarm auslösen
        return RM_TEST_ALARM;
    }

    if ((!setAlarmBus && alarmBus) || (!setTestAlarmBus && testAlarmBus))
    {
        // Alarm und Testalarm beenden
        return RM_NO_ALARM;
    }

    return RM_NO_CHANGE;
}

void SmokeDetectorAlarm::timerEverySecond()
{
    // Alarm: verzögert und zyklisch senden
    if (alarmLocal)
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

                groupObjects->write(GroupObject::grpObjAlarmBus, alarmLocal);
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
    if (testAlarmLocal)
    {
        if (config->testAlarmSendStatusPeriodically())
        {
            --TalarmCounter;
            if (!TalarmCounter)
            {
                TalarmCounter = config->testAlarmIntervalSeconds();
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
    if (ignoreBusAlarm & !(alarmBus | testAlarmBus))
    {
        ignoreBusAlarm = 0;
        groupObjects->setValue(GroupObject::grpObjResetAlarm, ignoreBusAlarm);
    }
}
