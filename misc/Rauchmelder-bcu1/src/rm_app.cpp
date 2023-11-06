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
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <stdint.h>
#include <type_traits>

#include "rm_app.h"
#include "rm_const.h"
#include "smoke_detector_alarm.h"
#include "smoke_detector_config.h"
#include "smoke_detector_device.h"
#include "smoke_detector_errorcode.h"
#include "smoke_detector_group_objects.h"

BCU1 bcu = BCU1();
SmokeDetectorConfig *config = new SmokeDetectorConfig(bcu.userEeprom);
SmokeDetectorGroupObjects *groupObjects = new SmokeDetectorGroupObjects(bcu.comObjects);
SmokeDetectorAlarm *alarm = new SmokeDetectorAlarm(config, groupObjects);
SmokeDetectorErrorCode *errorCode = new SmokeDetectorErrorCode(groupObjects); //!< Smoke detector error code handling
SmokeDetectorDevice *device = new SmokeDetectorDevice(config, groupObjects, alarm, errorCode);

unsigned char infoCounter;         //!< Countdown Zähler für zyklisches Senden der (Info) Com-Objekte
GroupObject infoSendObjno;         //!< Com-Objekt, das bei zyklischem Info Senden als nächstes geprüft/gesendet wird
unsigned char readCmdno;           //!< Nummer des Befehls, welcher als nächstes zyklisch an den Rauchmelder gesendet wird
unsigned char eventTime = DEFAULT_EVENTTIME; //!< Halbsekunden Zähler 0..119

/**
 * For description see declaration in file @ref rm_com.h
 */
void rm_process_msg(uint8_t *bytes, int8_t len)
{
    device->rm_process_msg(bytes, len);
}

/**
 * Empfangenes write_value_request Telegramm verarbeiten
 *
 * @param objno - Nummer des betroffenen Kommunikations-Objekts
 */
void objectUpdated(int objno)
{
    if (objno == GroupObject::grpObjAlarmBus ||         // Alarm Network
        objno == GroupObject::grpObjTestAlarmBus ||     // Test Alarm Network
        objno == GroupObject::grpObjResetAlarm)         // Alarm Reset
    {
        alarm->groupObjectUpdated(static_cast<GroupObject>(objno));
    }
}

/**
 * Den Zustand der Alarme bearbeiten. Wenn wir der Meinung sind der Bus-Alarm soll einen
 * bestimmten Zustand haben dann wird das dem Rauchmelder so lange gesagt bis der auch
 * der gleichen Meinung ist.
 */
void process_alarm_stats()
{
    if (device->hasOngoingMessageExchange()) ///\todo while waiting a answer we don't process alarms? rly?
    {
       return;
    }

    auto alarmState = alarm->loopCheckAlarmState();
    if (alarmState != RM_NO_CHANGE)
    {
        device->set_alarm_state(alarmState);
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

    --eventTime;

    device->timerEvery500ms();

    // Alles danach wird nur jede Sekunde gemacht
    if (eventTime & 1)
        return;

    alarm->timerEverySecond();

    // Jede zweite Sekunde ein Status Com-Objekt senden.
    // (Vormals war es jede 4. Sekunde, aber dann reicht 1 Minute nicht für 16 eventuell zu sendende Status Objekte (ComOject 6 - 21))
    // Aber nur senden wenn kein Alarm anliegt.
    if ((eventTime & DEFAULT_KNX_OBJECT_TIME) == 0 && infoSendObjno && !alarm->hasAlarm())
    {
        // Info Objekt zum Senden vormerken wenn es dafür konfiguriert ist.
        if (config->infoSendPeriodically(infoSendObjno))
        {
            groupObjects->send(infoSendObjno);
        }

        infoSendObjno = static_cast<GroupObject>(static_cast<std::underlying_type_t<GroupObject>>(infoSendObjno) - 1);
    }

    // alle 8 Sekunden einen der 6 Befehle aus der CmdTab an den Rauchmelder senden, um alle Status Daten aus dem Rauchmelder abzufragen
    // notwendig, da die ARM sblib keine Funktion aufruft, wenn ein Objekt ausgelesen wird
    // daher müssen alle Informationen immer im Speicher vorliegen
    if ((eventTime & DEFAULT_SERIAL_COMMAND_TIME) == 0 && readCmdno && !alarm->hasAlarm())
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

    if (!eventTime) // einmal pro Minute
    {
        eventTime = DEFAULT_EVENTTIME;

        alarm->timerEveryMinute();

        if (!readCmdno)
        {
            readCmdno = device->commandTableSize();
        }

        // Status Informationen zyklisch senden
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

void setupPeriodicTimer(uint32_t milliseconds)
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
 * Alle Applikations-Parameter zurücksetzen
 */
void initApplication()
{
    infoSendObjno = GroupObject::grpObjAlarmBus;
    readCmdno = device->commandTableSize();
    infoCounter = config->infoIntervalMinutes();
}
