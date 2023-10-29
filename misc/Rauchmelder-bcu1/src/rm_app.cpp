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

#include <smoke_detector_errorcode.h>
#include <bitset>
#include <cstring>
#include <stdint.h>

#include "rm_app.h"
#include "rm_const.h"
#include "rm_com.h"
#include "smoke_detector_config.h"

BCU1 bcu = BCU1();
SmokeDetectorConfig *config = new SmokeDetectorConfig(bcu.userEeprom);

//-----------------------------------------------------------------------------
// Befehle an den Rauchmelder
//-----------------------------------------------------------------------------
enum class Command : uint8_t
{
    rmSerialNumber          = 0,  //!< Gira Command: Seriennummer abfragen
    rmOperatingTime         = 1,  //!< Gira Command: Betriebszeit abfragen
    rmSmokeboxData          = 2,  //!< Gira Command: Rauchkammer Daten abfragen
    rmBatteryAndTemperature = 3,  //!< Gira Command: Batteriespannung und Temperaturen
    rmNumberAlarms_1        = 4,  //!< Gira Command: Anzahl der Alarme #1 abfragen
    rmNumberAlarms_2        = 5,  //!< Gira Command: Anzahl der Alarme #2 abfragen
    rmStatus                = 6,  //!< Gira Command: Status abfragen

    internal                = 14, //!< intern behandelt, kein Befehl an den Rauchmelder
};

/**
 * @ref RmCommandByte to multiple @ref GroupObject association table
 */
struct AssociationTable
{
    const RmCommandByte rmCommand;      //!< @ref RmCommandByte command to be sent
    const int8_t responseLength;        //!< Expected response length in bytes
    const uint8_t objects[MAX_OBJ_CMD]; //!< Association of the command to multiple @ref GroupObject. Use @ref grpObjInvalid to not associate
    uint32_t objValues;                 //!< Raw serial bytes of the @GroupObject
}
CmdTab[] =
{
    // CommandByte                  Response length         Object number   Raw value  Example response
    {RmCommandByte::serialNumber,           5, {GroupObject::grpObjSerialNumber,          // <STX>C4214710F31F<ETX>
                                                GroupObject::grpObjInvalid,
                                                GroupObject::grpObjInvalid,
                                                GroupObject::grpObjInvalid}, 0},
    {RmCommandByte::operatingTime,          5, {GroupObject::grpObjOperatingTime,         // <STX>C9000047E31F<ETX>
                                                GroupObject::grpObjInvalid,
                                                GroupObject::grpObjInvalid,
                                                GroupObject::grpObjInvalid}, 0},
    {RmCommandByte::smokeboxData,           5, {GroupObject::grpObjSmokeboxValue,         // <STX>CB0065000111<ETX>
                                                GroupObject::grpObjSmokeboxPollution,
                                                GroupObject::grpObjCountSmokeAlarm,
                                                GroupObject::grpObjInvalid}, 0},
    {RmCommandByte::batteryTemperatureData, 5, {GroupObject::grpObjBatteryVoltage,        // <STX>CC000155551B<ETX>
                                                GroupObject::grpObjTemperature,
                                                GroupObject::grpObjInvalid,
                                                GroupObject::grpObjInvalid}, 0},
    {RmCommandByte::numberAlarms_1,         5, {GroupObject::grpObjCountTemperatureAlarm, // <STX>CD0000000007<ETX>
                                                GroupObject::grpObjCountTestAlarm,
                                                GroupObject::grpObjCountAlarmWire,
                                                GroupObject::grpObjCountAlarmBus}, 0},
    {RmCommandByte::numberAlarms_2,         3, {GroupObject::grpObjCountTestAlarmWire,    // <STX>CE000048<ETX>
                                                GroupObject::grpObjCountTestAlarmBus,
                                                GroupObject::grpObjInvalid,
                                                GroupObject::grpObjInvalid}, 0},
    {RmCommandByte::status,                 5, {GroupObject::grpObjInvalid,               // <STX>C220000000F7<ETX>
                                                GroupObject::grpObjInvalid,
                                                GroupObject::grpObjInvalid,
                                                GroupObject::grpObjInvalid}, 0}
};

/**
 * Mapping of the @ref Commands to the smoke detector's response data (bytes).\
 * The index of @ref objMappingTab the table is the ID of the communication object (object id).
 */
constexpr struct
{
    Command cmd;            //!< @ref Command to send to the smoke-detector
    uint8_t offset;         //!< Byte-offset in the response
    uint8_t dataType;       //!< Datatype of the response
} objMappingTab[NUM_OBJS] =
{
    {Command::internal,                0, RM_TYPE_NONE},  //!<  0 @ref OBJ_ALARM_BUS
    {Command::internal,                0, RM_TYPE_NONE},  //!<  1 @ref OBJ_TALARM_BUS
    {Command::internal,                0, RM_TYPE_NONE},  //!<  2 @ref OBJ_RESET_ALARM
    {Command::internal,                0, RM_TYPE_NONE},  //!<  3 @ref OBJ_STAT_ALARM
    {Command::internal,                0, RM_TYPE_NONE},  //!<  4 @ref OBJ_STAT_ALARM_DELAYED
    {Command::internal,                0, RM_TYPE_NONE},  //!<  5 @ref OBJ_STAT_TALARM
    {Command::rmSerialNumber,          0, RM_TYPE_LONG},  //!<  6 @ref OBJ_SERIAL
    {Command::rmOperatingTime,         0, RM_TYPE_QSEC},  //!<  7 @ref OBJ_OPERATING_TIME
    {Command::rmSmokeboxData,          0, RM_TYPE_SHORT}, //!<  8 @ref OBJ_SMOKEBOX_VALUE
    {Command::rmSmokeboxData,          3, RM_TYPE_BYTE},  //!<  9 @ref OBJ_POLLUTION
    {Command::rmBatteryAndTemperature, 0, RM_TYPE_MVOLT}, //!< 10 @ref OBJ_BAT_VOLTAGE
    {Command::rmBatteryAndTemperature, 2, RM_TYPE_TEMP},  //!< 11 @ref OBJ_TEMP
    {Command::internal,                0, RM_TYPE_NONE},  //!< 12 @ref OBJ_ERRCODE
    {Command::internal,                0, RM_TYPE_NONE},  //!< 13 @ref OBJ_BAT_LOW
    {Command::internal,                0, RM_TYPE_NONE},  //!< 14 @ref OBJ_MALFUNCTION
    {Command::rmSmokeboxData,          2, RM_TYPE_BYTE},  //!< 15 @ref OBJ_CNT_SMOKEALARM
    {Command::rmNumberAlarms_1,        0, RM_TYPE_BYTE},  //!< 16 @ref OBJ_CNT_TEMPALARM
    {Command::rmNumberAlarms_1,        1, RM_TYPE_BYTE},  //!< 17 @ref OBJ_CNT_TESTALARM
    {Command::rmNumberAlarms_1,        2, RM_TYPE_BYTE},  //!< 18 @ref OBJ_CNT_ALARM_WIRE
    {Command::rmNumberAlarms_1,        3, RM_TYPE_BYTE},  //!< 19 @ref OBJ_CNT_ALARM_BUS
    {Command::rmNumberAlarms_2,        0, RM_TYPE_BYTE},  //!< 20 @ref OBJ_CNT_TALARM_WIRE
    {Command::rmNumberAlarms_2,        1, RM_TYPE_BYTE}   //!< 21 @ref OBJ_CNT_TALARM_BUS
};

void sendErrorCodeOnChange(bool batteryLowChanged, bool malfunctionChanged);

bool alarmLocal;                   //!< Flag für lokalen Alarm und Wired Alarm (über grüne Klemme / Rauchmelderbus)
bool alarmBus;                     //!< Flag für remote Alarm über EIB
bool testAlarmLocal;               //!< Flag für lokalen Testalarm und Wired Testalarm
bool testAlarmBus;                 //!< Flag für remote Testalarm über EIB
bool setAlarmBus;                  //!< Flag für den gewünschten Alarm Status wie wir ihn über den EIB empfangen haben
bool setTestAlarmBus;              //!< Flag für den gewünschten Testalarm Status wie wir ihn über den EIB empfangen haben
bool ignoreBusAlarm;               //!< Flag für Bus Alarm & -Testalarm ignorieren
SmokeDetectorErrorCode *errorCode = new SmokeDetectorErrorCode(sendErrorCodeOnChange); //!< Smoke detector error code handling
std::bitset<NUM_OBJS> objSendReqFlags;//!< Flags für Com-Objekte senden
unsigned char answerWait;          //!< Wenn != 0, dann Zähler für die Zeit die auf eine Antwort vom Rauchmelder gewartet wird.
#define INITIAL_ANSWER_WAIT 6      //!< Initialwert für answerWait in 0,5s
unsigned char alarmCounter;        //!< Countdown Zähler für zyklisches Senden eines Alarms.
unsigned char TalarmCounter;       //!< Countdown Zähler für zyklisches Senden eines Testalarms.
unsigned char delayedAlarmCounter; //!< Countdown Zähler für verzögertes Senden eines Alarms
unsigned char infoCounter;         //!< Countdown Zähler für zyklisches Senden der (Info) Com-Objekte
unsigned char infoSendObjno;       //!< Nummer des Com-Objekts das bei zyklischem Info Senden als nächstes geprüft/gesendet wird
unsigned char readCmdno;           //!< Nummer des Befehls, welcher als nächstes zyklisch an den Rauchmelder gesendet wird
unsigned char eventTime = DEFAULT_EVENTTIME; //!< Halbsekunden Zähler 0..119

/**
 * Den Alarm Status auf den Bus senden falls noch nicht gesendet.
 *
 * @param newAlarm - neuer Alarm Status
 *//*
void send_obj_alarm(bool newAlarm)
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
void send_obj_test_alarm(bool newAlarm)
{
    if (testAlarmLocal != newAlarm)
    {
        bcu.comObjects->objectWrite(GroupObject::grpObjTestAlarmBus, newAlarm);
        bcu.comObjects->objectWrite(GroupObject::grpObjStatusTestAlarm, newAlarm);
    }
}

void failHardInDebug() ///\todo remove on release
{
#ifdef DEBUG
    fatalError();
#endif
}

uint8_t commandTableSize()
{
    return sizeof(CmdTab)/sizeof(CmdTab[0]);
}

/**
 * Send error code, battery low, and malfunction groupobjects to the bus
 *
 * @param batteryLowChanged  Whether the low battery condition changed and therefore the corresponding group object needs to be sent
 * @param malfunctionChanged Whether the malfunction condition changed and therefore the corresponding group object needs to be sent
 */
void sendErrorCodeOnChange(bool batteryLowChanged, bool malfunctionChanged)
{
    bcu.comObjects->objectWrite(GroupObject::grpObjErrorCode, errorCode->code());

    if (batteryLowChanged)
    {
        bcu.comObjects->objectWrite(GroupObject::grpObjBatteryLow, errorCode->batteryLow());
    }

    if (malfunctionChanged)
    {
        bcu.comObjects->objectWrite(GroupObject::grpObjMalfunction, errorCode->malfunction());
    }
}

/**
 * Anhand der Kommunikationsobjektnummer die passenden Daten herausgeben
 */
unsigned long read_obj_value(unsigned char objno);

/**
 * For description see declaration in file @ref rm_com.h
 */
void rm_process_msg(uint8_t *bytes, int8_t len)
{
    uint8_t cmd;
    uint8_t msgType;

    answerWait = 0;
    errorCode->communicationTimeout(false);

    msgType = bytes[0];
    if (msgType == (RmCommandByte::status | 0x80))
    {
        msgType = (RmCommandByte::status | 0xc0); // "cast" automatic status message to "normal" status message
    }

    if ((msgType & 0xf0) != 0xc0) // check for valid response byte
    {
                // receiving an "echo" of our own command (e.g. <STX>0262<ETX>) can bring us here.
        return; // learned this by accidently touching the ARM's tx/rx pins
    }

    msgType &= 0x0f;
    for (cmd = 0; cmd < commandTableSize(); ++cmd)
    {
        if (CmdTab[cmd].rmCommand == msgType)
        {
            break;
        }
    }

    if (cmd >= commandTableSize())
    {
        failHardInDebug(); // found a new command/response => time to implement it
        return;
    }

    if (len != CmdTab[cmd].responseLength)
    {
        failHardInDebug(); // received length doesn't match expected length
        return;
    }

    if (RmCommandByte::status != msgType)
    {
        if ((len - 1) > (int8_t)sizeof(CmdTab[cmd].objValues))
        {
            failHardInDebug(); // response is to long to fit in .objValues
            return;
        }

        // Copy values over atomically.
        timer32_0.noInterrupts();
        CmdTab[cmd].objValues = 0;
        memcpy(&CmdTab[cmd].objValues, &bytes[1], len - 1);
        timer32_0.interrupts();

        // Informationen aus den empfangenen Daten vom Rauchmelder der sblib zur Verfügung stellen
        // Dazu alle Com-Objekte suchen auf die die empfangenen Daten passen (mapping durch CmdTab)
        // notwendig für den Abruf von Informationen über KNX aus den Status Objekten (GroupValueRead -> GroupValueResponse)
        for (unsigned char cmdObj_cnt = 0; (CmdTab[cmd].objects[cmdObj_cnt] != GroupObject::grpObjInvalid) &&
                                           (cmdObj_cnt < MAX_OBJ_CMD); cmdObj_cnt++)
        {
            uint8_t objno = CmdTab[cmd].objects[cmdObj_cnt];
            bcu.comObjects->objectSetValue(objno, read_obj_value(objno));
        }
    }
    else // status command gets special treatment
    {
        unsigned char subType = bytes[1];
        bool newAlarm;

        // (Alarm) Status

        unsigned char status = bytes[2];

        // Lokaler Alarm: Rauch Alarm | Temperatur Alarm | Wired Alarm
        newAlarm = (subType & 0x10) | (status & (0x04 | 0x08));
        if (config->alarmSendDelayed() && newAlarm) // wenn Alarm verzögert gesendet werden soll und Alarm ansteht
        {
            delayedAlarmCounter = config->alarmDelaySeconds();
            bcu.comObjects->objectSetValue(GroupObject::grpObjStatusAlarmDelayed, read_obj_value(GroupObject::grpObjStatusAlarmDelayed));
        }
        else if (alarmLocal != newAlarm) //wenn Alarm nicht verzögert gesendet werden soll oder Alarm nicht mehr ansteht (nur 1x senden)
        {
            bcu.comObjects->objectWrite(GroupObject::grpObjAlarmBus, newAlarm);
        }

        if (alarmLocal != newAlarm) //sobald neuer AlarmStatus ansteht, soll dieser versendet werden
        {
            bcu.comObjects->objectWrite(GroupObject::grpObjStatusAlarm, newAlarm);
        }

        alarmLocal = newAlarm;

        // Lokaler Testalarm: (lokaler) Testalarm || Wired Testalarm
        newAlarm = status & (0x20 | 0x40);
        send_obj_test_alarm(newAlarm);
        testAlarmLocal = newAlarm;

        // Bus Alarm
        alarmBus = status & 0x10;

        // Bus Testalarm
        testAlarmBus = status & 0x80;

        // Battery low
        bool batteryLow = ((status & 0x01) == 1);
        errorCode->batteryLow(batteryLow);

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

        if (subType & 0x08)  // Taste am Rauchmelder gedrückt
        {
            if (setAlarmBus) //wenn Alarm auf Bus anliegt
            {
                setAlarmBus = 0;
                delayedAlarmCounter = 0; // verzögerten Alarm abbrechen
                //objectWrite(GroupObject::grpObjStatusAlarm, read_obj_value(GroupObject::grpObjStatusAlarm));
            }

            if (setTestAlarmBus) //wenn Testalarm auf Bus anliegt
            {
                setTestAlarmBus = 0;
                //objectWrite(GroupObject::grpObjStatusTestAlarm, read_obj_value(GroupObject::grpObjStatusTestAlarm));
            }
        }

        bool temperatureSensor_1_fault = false;
        bool temperatureSensor_2_fault = false;

        if (subType & 0x02) // general smoke detector fault is indicated in 1. byte bit 1
        {
            // details are in 4. byte
            temperatureSensor_1_fault = (bytes[4] & 0x04); // sensor 1 state in 4. byte 2. bit
            temperatureSensor_2_fault = (bytes[4] & 0x10); // sensor 2 state in 4. byte 4. bit
        }

        errorCode->temperature_1_fault(temperatureSensor_1_fault);
        errorCode->temperature_2_fault(temperatureSensor_2_fault);

        ///\todo handle smoke box fault
        ///
    }
}

/**
 * Die Rauchmelder Antwort als Long Zahl liefern.
 *
 * @param cvalue - das erste Byte der Rauchmelder Antwort.
 * @return Der Wert mit getauschten Bytes.
 */
unsigned long answer_to_long(unsigned char *cvalue)
{
    return (cvalue[0] << 24) | (cvalue[1] << 16) | (cvalue[2] << 8) | cvalue[3];
}

/**
 * Die Rauchmelder Antwort als Integer Zahl liefern.
 *
 * @param cvalue - das erste Byte der Rauchmelder Antwort.
 * @return Der Wert mit getauschten Bytes.
 */
unsigned short answer_to_short(unsigned char *cvalue)
{
    return (cvalue[0] << 8) | cvalue[1];
}

/**
 * Wert eines Com-Objekts liefern.
 *
 * @param objno - die ID des Kommunikations-Objekts
 * @return Den Wert des Kommunikations Objekts
 */
unsigned long read_obj_value(unsigned char objno)
{
    Command cmd = objMappingTab[objno].cmd;

    // Interne Com-Objekte behandeln
    if (cmd == Command::internal)
    {
        switch (objno)
        {
            case GroupObject::grpObjAlarmBus:
            case GroupObject::grpObjStatusAlarm:
                return alarmLocal;

            case GroupObject::grpObjTestAlarmBus:
            case GroupObject::grpObjStatusTestAlarm:
                return testAlarmLocal;

            case GroupObject::grpObjResetAlarm:
                return ignoreBusAlarm;

            case GroupObject::grpObjStatusAlarmDelayed:
                return delayedAlarmCounter != 0;

            case GroupObject::grpObjBatteryLow:
                return errorCode->batteryLow();

            case GroupObject::grpObjMalfunction:
                return errorCode->malfunction();

            case GroupObject::grpObjErrorCode:
                return errorCode->code();

            default:
                return -1; // Fehler: unbekanntes Com Objekt
        }
    }
    // Com-Objekte verarbeiten die Werte vom Rauchmelder darstellen
    else
    {
        unsigned long lval;
        unsigned char *answer;

        answer = (unsigned char*) &CmdTab[(uint8_t)cmd].objValues;
        answer += objMappingTab[objno].offset;

        switch (objMappingTab[objno].dataType)
        {
            case RM_TYPE_BYTE:
                return *answer;

            case RM_TYPE_LONG:
                return answer_to_long(answer);

            case RM_TYPE_QSEC:  // Betriebszeit verarbeiten
                lval = answer_to_long(answer) >> 2; // Wert in Sekunden
                if (config->infoSendOperationTimeInHours())
                    return lval / 3600; // Stunden, 16Bit
                else
                    return lval;        // Sekunden, 32Bit

            case RM_TYPE_SHORT:
                return answer_to_short(answer);

            case RM_TYPE_TEMP:
                // Conversion per temp sensor: (answer[x] * 0.5°C - 20°C) * 100 [for DPT9]
                lval = ((int) answer[0]) + answer[1];
                lval *= 25; // in lval sind zwei Temperaturen, daher halber Multiplikator
                lval -= 2000;
                lval += config->temperatureOffsetInTenthDegrees() * 10;  // Temperaturabgleich
                return (floatToDpt9(lval));

            case RM_TYPE_MVOLT:
                if ((answer[0] == 0) && (answer[1] == 1))
                {
                    return (floatToDpt9(BATTERY_VOLTAGE_INVALID));
                }
                lval = answer_to_short(answer);
                // Conversion: lval * 5.7 * 3.3V / 1024 * 1000mV/V * 100 [for DPT9]
                lval *= 9184;
                lval /= 5;
                return (floatToDpt9(lval));

            default: // Fehler: unbekannter Datentyp
                return -2;
        }
    }
}

/**
 * Empfangenes write_value_request Telegramm verarbeiten
 *
 * @param objno - Nummer des betroffenen Kommunikations-Objekts
 */
void objectUpdated(int objno)
{
    if (objno == GroupObject::grpObjAlarmBus) // Bus Alarm
    {
        setAlarmBus = bcu.comObjects->objectRead(objno) & 0x01; //ToDo: prüfen ob ok   //war: setAlarmBus = telegramm[7] & 0x01;

        // Wenn wir lokalen Alarm haben dann Bus Alarm wieder auslösen
        // damit der Status der anderen Rauchmelder stimmt
        if (!setAlarmBus && alarmLocal)
            bcu.comObjects->objectWrite(GroupObject::grpObjAlarmBus, read_obj_value(GroupObject::grpObjAlarmBus)); //send_obj_value(GroupObject::alarmBus);

        if (ignoreBusAlarm)
            setAlarmBus = 0;
    }
    else if (objno == GroupObject::grpObjTestAlarmBus) // Bus Test Alarm
    {
        setTestAlarmBus = bcu.comObjects->objectRead(objno) & 0x01; //ToDo: prüfen ob ok   //war: setTestAlarmBus = telegramm[7] & 0x01;

        // Wenn wir lokalen Testalarm haben dann Bus Testalarm wieder auslösen
        // damit der Status der anderen Rauchmelder stimmt
        if (!setTestAlarmBus && testAlarmLocal)
            bcu.comObjects->objectWrite(GroupObject::grpObjTestAlarmBus, read_obj_value(GroupObject::grpObjTestAlarmBus)); //send_obj_value(GroupObject::grpObjTestAlarmBus);

        if (ignoreBusAlarm)
            setTestAlarmBus = 0;
    }
    else if (objno == GroupObject::grpObjResetAlarm) // Bus Alarm rücksetzen
    {
        setAlarmBus = 0;
        setTestAlarmBus = 0;
        ignoreBusAlarm = 1;
    }
}

/**
 * Enable/disable the 12V supply voltage
 *
 * @param enable     Set true to enable supply voltage, false to disable it
 * @param waitTimeMs Time in milliseconds to wait after supply voltage was enabled/disabled
 */
void setSupplyVoltageAndWait(bool enable, uint32_t waitTimeMs)
{
    pinMode(LED_SUPPLY_VOLTAGE_DISABLED_PIN, OUTPUT);
    digitalWrite(LED_SUPPLY_VOLTAGE_DISABLED_PIN, enable); // disable/enable led first to save/drain some juice

    // Running pinMode the first time, sets it by default to low
    // which will enable the support voltage and charge the capacitor.
    // So please put nothing in between pinMode and digitalWrite.
    pinMode(RM_SUPPORT_VOLTAGE_PIN, OUTPUT);
    if (enable)
    {
        digitalWrite(RM_SUPPORT_VOLTAGE_PIN, RM_SUPPORT_VOLTAGE_ON);
    }
    else
    {
        digitalWrite(RM_SUPPORT_VOLTAGE_PIN, RM_SUPPORT_VOLTAGE_OFF);
    }

    if (waitTimeMs != 0)
    {
        delay(waitTimeMs);
    }

    errorCode->supplyVoltageDisabled(!enable);
}

/**
 * Checks if the smoke detector is on the base plate and switches the supply voltage on
 */
void checkRmAttached2BasePlate(void)
{
    bool rmActive = (digitalRead(RM_ACTIVITY_PIN) == RM_IS_ACTIVE);
    digitalWrite(LED_BASEPLATE_DETACHED_PIN, rmActive);

    errorCode->coverPlateAttached(rmActive);

    if (digitalRead(RM_SUPPORT_VOLTAGE_PIN) == RM_SUPPORT_VOLTAGE_ON)
    {
        return; // supply voltage is already on
    }

    ///\todo check danger of this timeout. Can it show up as a working smoke detector on the bus?
    rmActive |= (millis() >= SUPPLY_VOLTAGE_TIMEOUT_MS);

    // der Rauchmelder wurde auf die Bodenplatte gesteckt => Spannungsversorgung aktivieren
    if (rmActive)
    {
        delay(RM_POWER_UP_TIME_MS);
        setSupplyVoltageAndWait(true, SUPPLY_VOLTAGE_ON_DELAY_MS);
        rm_serial_init(); //serielle Schnittstelle für die Kommunikation mit dem Rauchmelder initialisieren
    }
}

/**
 * Send command @ref cmd to smoke detector.\n
 * Receiving and processing the response from the smoke detector is done in @ref rm_process_msg().
 *
 * @param cmd - Index of the command to be send from the @ref CmdTab
 * @return True if command was sent, otherwise false.
 */
bool send_Cmd(Command cmd)
{
    checkRmAttached2BasePlate(); ///\todo If think this should be moved to TIMER32_0_IRQHandler

    if (isReceiving())
    {
        return false;
    }

    if (!rm_send_cmd(CmdTab[(uint8_t)cmd].rmCommand))
    {
        return false;
    }

    ///\todo setting objValues to invalid, should be done after a serial timeout occurred
    switch (cmd)
    {
        case Command::rmSerialNumber:
            break;

        case Command::rmOperatingTime:
            break;

        case Command::rmSmokeboxData:
            break;

        case Command::rmBatteryAndTemperature:
            CmdTab[(uint8_t)Command::rmBatteryAndTemperature].objValues = 0;
            break;

        case Command::rmNumberAlarms_1:
            break;

        case Command::rmNumberAlarms_2:
            break;

        default:
            break;
    }

    answerWait = INITIAL_ANSWER_WAIT;
    return true;
}

/**
 * Ein Com-Objekt bearbeiten.
 *
 * @param objno - die Nummer des zu bearbeitenden Com Objekts
 */
void process_obj(unsigned char objno)
{
    // Der Wert des Com-Objekts ist bekannt, also sofort senden
    // Die Werte werden zyklisch (minütlich) alle vom Rauchmelder abgefragt, daher sind immer alle Werte aktuell vorhanden

    if (objSendReqFlags[objno])
    {
        bcu.comObjects->objectWrite(objno, read_obj_value(objno));
        objSendReqFlags[objno] = false;
    }
}

/**
 * Com-Objekte bearbeiten.
 *
 * Com-Objekte, die Daten vom Rauchmelder benötigen, werden nur bearbeitet wenn
 * nicht gerade auf Antwort vom Rauchmelder gewartet wird.
 */
void process_objs()
{
    if (answerWait || objSendReqFlags.none())
    {
       return;
    }

    for (auto objno = 0; objno < objSendReqFlags.size(); ++objno)
    {
        process_obj(objno);
    }
}

/**
 * Den Zustand der Alarme bearbeiten. Wenn wir der Meinung sind der Bus-Alarm soll einen
 * bestimmten Zustand haben dann wird das dem Rauchmelder so lange gesagt bis der auch
 * der gleichen Meinung ist.
 */
void process_alarm_stats()
{
    if (answerWait) ///\todo while waiting a answer we don't process alarms? rly?
    {
       return;
    }

    if (setAlarmBus && !alarmBus)
    {
        // Alarm auslösen
        rm_set_alarm_state(RM_ALARM);
        answerWait = INITIAL_ANSWER_WAIT;
    }
    else if (setTestAlarmBus && !testAlarmBus)
    {
        // Testalarm auslösen
        rm_set_alarm_state(RM_TEST_ALARM);
        answerWait = INITIAL_ANSWER_WAIT;
    }
    else if ((!setAlarmBus && alarmBus) || (!setTestAlarmBus && testAlarmBus))
    {
        // Alarm und Testalarm beenden
        rm_set_alarm_state(RM_NO_ALARM);
        answerWait = INITIAL_ANSWER_WAIT;
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

    // Wir warten auf eine Antwort vom Rauchmelder
    if (answerWait)
    {
        --answerWait;
        if (!answerWait)
        {
            errorCode->communicationTimeout(true);
        }
    }

    // Alles danach wird nur jede Sekunde gemacht
    if (eventTime & 1)
        return;

    // Alarm: verzögert und zyklisch senden
    if (alarmLocal)
    {
        // Alarm verzögert senden
        if (delayedAlarmCounter)
        {
            --delayedAlarmCounter;
            if (!delayedAlarmCounter)   // Verzögerungszeit abgelaufen
            {
                bcu.comObjects->objectSetValue(GroupObject::grpObjStatusAlarmDelayed, read_obj_value(GroupObject::grpObjStatusAlarmDelayed)); // Status verzögerter Alarm zurücksetzen
                //objSendReqFlags[GroupObject::grpObjAlarmBus] = true;  // Vernetzung Alarm senden
                //objSendReqFlags[GroupObject::grpObjStatusAlarm] = true; // Status Alarm senden

                bcu.comObjects->objectWrite(GroupObject::grpObjAlarmBus, alarmLocal);
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
                        objSendReqFlags[GroupObject::grpObjAlarmBus] = true; // Vernetzung Alarm senden
                    }
                    objSendReqFlags[GroupObject::grpObjStatusAlarm] = true;
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
                objSendReqFlags[GroupObject::grpObjStatusAlarm] = true;
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
                    objSendReqFlags[GroupObject::grpObjTestAlarmBus] = true;
                }
                objSendReqFlags[GroupObject::grpObjStatusTestAlarm] = true;
            }
        }
    }

    // Jede zweite Sekunde ein Status Com-Objekt senden.
    // (Vormals war es jede 4. Sekunde, aber dann reicht 1 Minute nicht für 16 eventuell zu sendende Status Objekte (ComOject 6 - 21))
    // Aber nur senden wenn kein Alarm anliegt.
    if ((eventTime & DEFAULT_KNX_OBJECT_TIME) == 0 && infoSendObjno &&
        !(alarmLocal | alarmBus | testAlarmLocal | testAlarmBus))
    {
        // Info Objekt zum Senden vormerken wenn es dafür konfiguriert ist.
        if (config->infoSendPeriodically(static_cast<GroupObject>(infoSendObjno)))
        {
            objSendReqFlags[infoSendObjno] = true;
        }

        --infoSendObjno;
    }

    // alle 8 Sekunden einen der 6 Befehle aus der CmdTab an den Rauchmelder senden, um alle Status Daten aus dem Rauchmelder abzufragen
    // notwendig, da die ARM sblib keine Funktion aufruft, wenn ein Objekt ausgelesen wird
    // daher müssen alle Informationen immer im Speicher vorliegen
    if ((eventTime & DEFAULT_SERIAL_COMMAND_TIME) == 0 && readCmdno &&
        !(alarmLocal | alarmBus | testAlarmLocal | testAlarmBus))
    {
        if (!answerWait)
        {
            readCmdno--;
            if (!send_Cmd((Command)readCmdno))
            {
                readCmdno++;
            }
        }
    }

    if (!eventTime) // einmal pro Minute
    {
        eventTime = DEFAULT_EVENTTIME;

        // Bus Alarm ignorieren Flag rücksetzen wenn kein Alarm mehr anliegt
        if (ignoreBusAlarm & !(alarmBus | testAlarmBus))
            ignoreBusAlarm = 0;

        if (!readCmdno)
        {
            readCmdno = commandTableSize();
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
    // Werte initialisieren
    objSendReqFlags.reset();

    for (uint8_t i = 0; i < commandTableSize(); ++i)
    {
        CmdTab[i].objValues = 0;
    }

    answerWait = 0;

    alarmBus = 0;
    alarmLocal = 0;

    testAlarmBus = 0;
    testAlarmLocal = 0;

    setAlarmBus = 0;
    setTestAlarmBus = 0;
    ignoreBusAlarm = 0;

    infoSendObjno = 0;
    readCmdno = commandTableSize();
    infoCounter = config->infoIntervalMinutes();
    alarmCounter = 1;
    TalarmCounter = 1;
    delayedAlarmCounter = 0;

    errorCode->clearAllErrors();

    // set all comObjects to default
    for (uint8_t i = 0; i < NUM_OBJS; i++)
    {
        bcu.comObjects->objectSetValue(i, 0);
    }

    pinMode(LED_BASEPLATE_DETACHED_PIN, OUTPUT);
    digitalWrite(LED_BASEPLATE_DETACHED_PIN, false);
    pinMode(RM_ACTIVITY_PIN, INPUT | PULL_DOWN); // smoke detector base plate state, pulldown configured, Pin is connected to 3.3V VCC of the RM

    setSupplyVoltageAndWait(false, SUPPLY_VOLTAGE_OFF_DELAY_MS);  ///\todo move waiting to delayed app start, make sure it lasts at least 500ms to discharge the 12V capacitor
}
