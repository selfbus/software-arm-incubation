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
#include "rm_app.h"
#include "rm_const.h"
#include "rm_com.h"
#include "rm_eeprom.h"

#include <cstring>

BCU1 bcu = BCU1();

//-----------------------------------------------------------------------------
// Befehle an den Rauchmelder
//-----------------------------------------------------------------------------
enum class Command : uint8_t // RM_CMD_COUNT must match number of rmXxxx commands
{
    rmSerialNumber          = 0,  //!< Gira Command: Seriennummer abfragen
    rmOperatingTime         = 1,  //!< Gira Command: Betriebszeit abfragen
    rmSmokeboxData          = 2,  //!< Gira Command: Rauchkammer Daten abfragen
    rmBatteryAndTemperature = 3,  //!< Gira Command: Batteriespannung und Temperaturen
    rmNumberAlarms_1        = 4,  //!< Gira Command: Anzahl der Alarme #1 abfragen
    rmNumberAlarms_2        = 5,  //!< Gira Command: Anzahl der Alarme #2 abfragen
    rmStatus                = 6,  //!< Gira Command: Status abfragen

    internal                = 14, //!< intern behandelt, kein Befehl an den Rauchmelder
    none                    = 15  //!< kein Befehl
};

#define RM_CMD_COUNT           7  //!< Anzahl der Gira Commands

struct
{
    const RmCommandByte cmdno;                //!< Zu sendender RM_CMD Befehl
    const unsigned char objects[MAX_OBJ_CMD]; //!< Zuordnung der ComObjekte zu den Befehlen
    unsigned long objValues;                  //!< Werte der Com-Objekte.
} CmdTab[RM_CMD_COUNT] =
{
    // CommandByte                          Object number                                     Raw value  Example response
    {RmCommandByte::serialNumber,           {GroupObject::grpObjSerialNumber, 0xFF, 0xFF, 0xFF}, 0},  // <STX>C4214710F31F<ETX>
    {RmCommandByte::operatingTime,          {GroupObject::grpObjOperatingTime, 0xFF, 0xFF, 0xFF}, 0}, // <STX>C9000047E31F<ETX>
    {RmCommandByte::smokeboxData,           {GroupObject::grpObjSmokeboxValue,                        // <STX>CB0065000111<ETX>
                                             GroupObject::grpObjSmokeboxPollution,
                                             GroupObject::grpObjCountSmokeAlarm, 0xFF}, 0},
    {RmCommandByte::batteryTemperatureData, {GroupObject::grpObjBatteryVoltage,                       // <STX>CC000155551B<ETX>
                                             GroupObject::grpObjTemperature, 0xFF, 0xFF}, 0},
    {RmCommandByte::numberAlarms_1,         {GroupObject::grpObjCountTemperatureAlarm,                // <STX>CD0000000007<ETX>
                                             GroupObject::grpObjCountTestAlarm,
                                             GroupObject::grpObjCountAlarmWire,
                                             GroupObject::grpObjCountAlarmBus}, 0},
    {RmCommandByte::numberAlarms_2,         {GroupObject::grpObjCountTestAlarmWire,
                                             GroupObject::grpObjCountTestAlarmBus, 0xFF, 0xFF}, 0},   // <STX>CE000048<ETX>
    {RmCommandByte::status,                 {0xFF, 0xFF, 0xFF, 0xFF}, 0}                              // <STX>C220000000F7<ETX>
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

bool alarmLocal;                   //!< Flag für lokalen Alarm und Wired Alarm (über grüne Klemme / Rauchmelderbus)
bool alarmBus;                     //!< Flag für remote Alarm über EIB
bool testAlarmLocal;               //!< Flag für lokalen Testalarm und Wired Testalarm
bool testAlarmBus;                 //!< Flag für remote Testalarm über EIB
bool setAlarmBus;                  //!< Flag für den gewünschten Alarm Status wie wir ihn über den EIB empfangen haben
bool setTestAlarmBus;              //!< Flag für den gewünschten Testalarm Status wie wir ihn über den EIB empfangen haben
bool ignoreBusAlarm;               //!< Flag für Bus Alarm & -Testalarm ignorieren
unsigned char errCode;             //!< Rauchmelder Fehlercodes
unsigned char objSendReqFlags[NUM_OBJ_FLAG_BYTES];//!< Flags für Com-Objekte senden
unsigned char answerWait;          //!< Wenn != 0, dann Zähler für die Zeit die auf eine Antwort vom Rauchmelder gewartet wird.
#define INITIAL_ANSWER_WAIT 6      //!< Initialwert für answerWait in 0,5s
unsigned char noAnswerCount;       //!< Zähler für keine Antwort vom Rauchmelder
#define NO_ANSWER_MAX 5            //!< Maximale Anzahl in noAnswerCount ab der ein Rauchmelder Fehler gemeldet wird
unsigned char alarmCounter;        //!< Countdown Zähler für zyklisches Senden eines Alarms.
unsigned char TalarmCounter;       //!< Countdown Zähler für zyklisches Senden eines Testalarms.
unsigned char delayedAlarmCounter; //!< Countdown Zähler für verzögertes Senden eines Alarms
unsigned char infoCounter;         //!< Countdown Zähler für zyklisches Senden der (Info) Com-Objekte
unsigned char infoSendObjno;       //!< Nummer des Com-Objekts das bei zyklischem Info Senden als nächstes geprüft/gesendet wird
unsigned char readCmdno;           //!< Nummer des Befehls, welcher als nächstes zyklisch an den Rauchmelder gesendet wird
unsigned char eventTime = DEFAULT_EVENTTIME; //!< Halbsekunden Zähler 0..119
const unsigned char pow2[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };         //!< Tabelle für 1<<x, d.h. pow2[3] == 1<<3

#define ARRAY_SET_BIT(arr, bitno) arr[bitno>>3] |= pow2[bitno & 7]     //!< Im Byte Array arr das bitno-te Bit setzen
#define ARRAY_CLEAR_BIT(arr, bitno) arr[bitno>>3] &= ~pow2[bitno & 7]  //!< Im Byte Array arr das bitno-te Bit löschen
#define ARRAY_IS_BIT_SET(arr, bitno) (arr[bitno>>3] & pow2[bitno & 7]) //!< Testen ob im Byte Array arr das bitno-te Bit gesetzt ist

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

uint8_t commandTableSize()
{
    return sizeof(CmdTab)/sizeof(CmdTab[0]);
}

/**
 * Fehlercode setzen
 *
 * @param newErrCode - neuer Fehlercode
 */
void set_errcode(unsigned char newErrCode)
{
    if (newErrCode == errCode)
        return;

    // Wenn sich der Status der Batterie geändert hat dann GroupObject::batteryLow senden,
    // sonst den allgemeinen Fehler Indikator GroupObject::grpObjMalfunction.
    if ((errCode ^ newErrCode) & ERRCODE_BATLOW)
        ARRAY_SET_BIT(objSendReqFlags, GroupObject::grpObjBatteryLow);
    else
        ARRAY_SET_BIT(objSendReqFlags, GroupObject::grpObjMalfunction);

    errCode = newErrCode;
    ARRAY_SET_BIT(objSendReqFlags, GroupObject::grpObjErrorCode);
}

/**
 * Die empfangene Nachricht vom Rauchmelder verarbeiten.
 * Wird von _receive() aufgerufen.
 */
void rm_process_msg(unsigned char *bytes, unsigned char len)
{
    uint8_t cmd;
    uint8_t msgType;
    uint8_t byteno;
    uint8_t mask;

    answerWait = 0;
    if (noAnswerCount)
    {
        noAnswerCount = 0;
        set_errcode(errCode & ~ERRCODE_COMM);
    }

    msgType = bytes[0];
    if (msgType == (RmCommandByte::status | 0xc0))
    {
        msgType = (RmCommandByte::status | 0x80); // "cast" to automatic status message
    }

    if ((msgType & 0xf0) == 0xc0) // Com-Objekt Werte empfangen
    {
        msgType &= 0x0f;
        for (cmd = 0; cmd < commandTableSize(); ++cmd)
        {
            if (CmdTab[cmd].cmdno == msgType)
                break;
        }

        if (cmd < commandTableSize())
        {
            // Copy values over atomically.
            timer32_0.noInterrupts();
            CmdTab[cmd].objValues = 0;
            memcpy(&CmdTab[cmd].objValues, &bytes[1], len - 1);
            timer32_0.interrupts();

            // Informationen aus den empfangenen Daten vom Rauchmelder der sblib zur Verfügung stellen
            // Dazu alle Com-Objekte suchen auf die die empfangenen Daten passen (mapping durch CmdTab)
            // notwendig für den Abruf von Informationen über KNX aus den Status Objekten (GroupValueRead -> GroupValueResponse)
            for (unsigned char cmdObj_cnt = 0; CmdTab[cmd].objects[cmdObj_cnt] != 0xFF && cmdObj_cnt < MAX_OBJ_CMD; cmdObj_cnt++)
            {
                uint8_t objno = CmdTab[cmd].objects[cmdObj_cnt];
                bcu.comObjects->objectSetValue(objno, read_obj_value(objno));

                // Versand der erhaltenen Com-Objekte einleiten.
                // Sofern sie für den Versand vorgemerkt sind.
                byteno = objno >> 3;
                mask = pow2[objno & 7];

                if (objSendReqFlags[byteno] & mask)
                {
                    bcu.comObjects->objectWrite(objno, read_obj_value(objno));
                    objSendReqFlags[byteno] &= ~mask;
                }
            }
        }
    }
    else if (msgType == 0x82 && len == 5) // Status Meldung
    {
        unsigned char subType = bytes[1];
        bool newAlarm;

        // (Alarm) Status

        unsigned char status = bytes[2];

        // Lokaler Alarm: Rauch Alarm | Temperatur Alarm | Wired Alarm
        newAlarm = (subType & 0x10) | (status & (0x04 | 0x08));
        if ((bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_ALARM_DELAYED) && newAlarm) // wenn Alarm verzögert gesendet werden soll und Alarm ansteht
        {
            delayedAlarmCounter = bcu.userEeprom->getUInt8(CONF_ALARM_DELAYED);
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

        // Batterie schwach/leer
        if ((status ^ errCode) & ERRCODE_BATLOW)
        {
            set_errcode((errCode & ~ERRCODE_BATLOW) | (status & ERRCODE_BATLOW));
            // Werte für die sblib zur Verfügung stellen
            // notwendig für den Abruf von Informationen über KNX aus den Status Objekten (GroupValueRead -> GroupValueResponse)
            bcu.comObjects->objectSetValue(GroupObject::grpObjErrorCode, read_obj_value(GroupObject::grpObjErrorCode));
            bcu.comObjects->objectSetValue(GroupObject::grpObjBatteryLow, read_obj_value(GroupObject::grpObjBatteryLow));
            bcu.comObjects->objectSetValue(GroupObject::grpObjMalfunction, read_obj_value(GroupObject::grpObjMalfunction));
        }

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

        if (subType & 0x02)  // Defekt am Rauchmelder
        {
            unsigned char status = bytes[4];
            unsigned char newErrCode = errCode & (ERRCODE_BATLOW | ERRCODE_COMM);

            if (status & 0x04)
                newErrCode |= ERRCODE_TEMP1;

            if (status & 0x10)
                newErrCode |= ERRCODE_TEMP2;

            // TODO Rauchkammer Defekt behandeln

            set_errcode(newErrCode);
        }
    }
    else if (msgType == 0x82 && len > 5) // zu lange Status Meldung
    {
#ifdef DEBUG
        fatalError(); ///\todo we should never land here AND fatalError() can't be the final solution
#endif
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
                return (errCode & ERRCODE_BATLOW) != 0;

            case GroupObject::grpObjMalfunction:
                return (errCode & ~ERRCODE_BATLOW) != 0;

            case GroupObject::grpObjErrorCode:
                return errCode;

            default:
                return -1; // Fehler: unbekanntes Com Objekt
        }
    }
    // Com-Objekte verarbeiten die Werte vom Rauchmelder darstellen
    else if (cmd != Command::none)
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
                if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_OPERATION_TIME_TYPE)
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
                lval += (signed char) bcu.userEeprom->getUInt8(CONF_TEMP_OFFSET) * 10;  // Temperaturabgleich
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

    // Fehler: unbekanntes Com Objekt
    return -1;
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
 * Checks if the smoke detector is on the base plate and switches the supply voltage on
 */
void checkRmAttached2BasePlate(void)
{
    bool rmActiv = digitalRead(RM_ACTIVITY_PIN);
    digitalWrite(LED_BASEPLATE_DETACHED_PIN, rmActiv);

    if (digitalRead(RM_SUPPORT_VOLTAGE_PIN) == RM_SUPPORT_VOLTAGE_ON)
    {
        return; // supply voltage is already on
    }

    // der Rauchmelder wurde auf die Bodenplatte gesteckt => Spannungsversorgung aktivieren
    if ((rmActiv == RM_IS_ACTIVE) || (millis() >= SUPPLY_VOLTAGE_TIMEOUT_MS))
    {
        digitalWrite(RM_SUPPORT_VOLTAGE_PIN, RM_SUPPORT_VOLTAGE_ON); // Spannungsversorgung aktivieren
        delay(SUPPLY_VOLTAGE_ON_DELAY_MS);
        pinMode(RM_COMM_ENABLE_PIN, OUTPUT);
        digitalWrite(RM_COMM_ENABLE_PIN, RM_COMM_ENABLE); // Kommunikation mit dem RM aktivieren
        digitalWrite(LED_SUPPLY_VOLTAGE_DISABLED_PIN, true);
    }
}

/**
 * Befehl an den Rauchmelder versenden
 * Der Empfang und die Verarbeitung der Antwort des Rauchmelders erfolgt in process_msg().
 *
 * @param cmd - Index des zu sendenden Befehls aus der CmdTab
 */
void send_Cmd(Command cmd)
{
    if (isReceiving())
    {
        return;
    }

    checkRmAttached2BasePlate();

    ///\todo set remaining objValues to invalid values before sending serial command
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

    rm_send_cmd(CmdTab[(uint8_t)cmd].cmdno);
    answerWait = INITIAL_ANSWER_WAIT;
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

    unsigned char byteno = objno >> 3;
    unsigned char mask = pow2[objno & 7];

    if (objSendReqFlags[byteno] & mask)
    {
        bcu.comObjects->objectWrite(objno, read_obj_value(objno));
        objSendReqFlags[byteno] &= ~mask;
    }
}

/**
 * Com-Objekte bearbeiten, Worker Funktion.
 *
 * Com-Objekte, die Daten vom Rauchmelder benötigen, werden nur bearbeitet wenn
 * nicht gerade auf Antwort vom Rauchmelder gewartet wird.
 *
 * @return 1 wenn ein Com-Objekt verarbeitet wurde, sonst 0.
 */
unsigned char do_process_objs(unsigned char *flags)
{
    unsigned char byteno, bitno, objno, flagsByte;
    Command cmd;

    for (byteno = 0; byteno < NUM_OBJ_FLAG_BYTES; ++byteno)
    {
        flagsByte = flags[byteno];
        if (!flagsByte)
            continue;

        for (bitno = 0; bitno < 8; ++bitno)
        {
            if (flagsByte & pow2[bitno])
            {
                objno = (byteno << 3) + bitno;
                cmd = objMappingTab[objno].cmd;
                if (!answerWait || cmd == Command::none || cmd == Command::internal)
                {
                    process_obj(objno);
                    return 1;
                }
            }
        }
    }

    return 0;
}

/**
 * Com-Objekte bearbeiten.
 */
void process_objs()
{
    do_process_objs(objSendReqFlags);
}

/**
 * Den Zustand der Alarme bearbeiten. Wenn wir der Meinung sind der Bus-Alarm soll einen
 * bestimmten Zustand haben dann wird das dem Rauchmelder so lange gesagt bis der auch
 * der gleichen Meinung ist.
 */
void process_alarm_stats()
{
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

        // Wenn keine Antwort vom Rauchmelder kommt dann den noAnswerCount Zähler
        // erhöhen. Wenn der Zähler NO_ANSWER_MAX erreicht dann ist es ein Rauchmelder
        // Fehler.
        if (!answerWait)
        {
            if (noAnswerCount < 255)
            {
                ++noAnswerCount;
            }
            if (noAnswerCount >= NO_ANSWER_MAX)
            {
                set_errcode(errCode | ERRCODE_COMM);
            }
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
                //ARRAY_SET_BIT(objSendReqFlags, GroupObject::grpObjAlarmBus);  // Vernetzung Alarm senden
                //ARRAY_SET_BIT(objSendReqFlags, GroupObject::grpObjStatusAlarm); // Status Alarm senden

                bcu.comObjects->objectWrite(GroupObject::grpObjAlarmBus, alarmLocal);
            }
        }
        else // Alarm zyklisch senden
        {
            if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_ALARM_INTERVAL)
            {
                --alarmCounter;
                if (!alarmCounter)
                {
                    alarmCounter = bcu.userEeprom->getUInt8(CONF_ALARM_INTERVAL);     // Zykl. senden Zeit holen
                    if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_ALARM_INTERVAL_NW)
                    {
                        ARRAY_SET_BIT(objSendReqFlags, GroupObject::grpObjAlarmBus); // Vernetzung Alarm senden
                    }
                    ARRAY_SET_BIT(objSendReqFlags, GroupObject::grpObjStatusAlarm);
                }
            }
        }
    }
    // Kein Alarm, zyklisch 0 senden
    else
    {
        if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_TALARM_INTERVAL_S0)
        {
            --alarmCounter;
            if (!alarmCounter)
            {
                alarmCounter = bcu.userEeprom->getUInt8(CONF_ALARM_INTERVAL); // Zykl. senden Zeit holen
                ARRAY_SET_BIT(objSendReqFlags, GroupObject::grpObjStatusAlarm);
            }
        }
    }

    // Testalarm: zyklisch senden
    if (testAlarmLocal)
    {
        if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_TALARM_INTERVAL)
        {
            --TalarmCounter;
            if (!TalarmCounter)
            {
                TalarmCounter = bcu.userEeprom->getUInt8(CONF_TALARM_INTERVAL);
                if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_TALARM_INTERVAL_NW)
                {
                    ARRAY_SET_BIT(objSendReqFlags, GroupObject::grpObjTestAlarmBus);
                }
                ARRAY_SET_BIT(objSendReqFlags, GroupObject::grpObjStatusTestAlarm);
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
        // Leider sind die Bits in der VD in der falschen Reihenfolge, daher 7-x
        if ((infoSendObjno >= 14 && (bcu.userEeprom->getUInt8(CONF_INFO_14TO21) & pow2[7 - (infoSendObjno - 14)])) ||
            (infoSendObjno < 14 && infoSendObjno >= 6 && (bcu.userEeprom->getUInt8(CONF_INFO_6TO13) & pow2[7 - (infoSendObjno - 6)])))
        {
            ARRAY_SET_BIT(objSendReqFlags, infoSendObjno);
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
            send_Cmd((Command)readCmdno);
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
        if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_INFO_INTERVAL)
        {
            --infoCounter;
            if (!infoCounter)
            {
                infoCounter = bcu.userEeprom->getUInt8(CONF_INFO_INTERVAL);
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
    rm_serial_init(); //serielle Schnittstelle für die Kommunikation mit dem Rauchmelder initialisieren

    // Werte initialisieren
    for (uint8_t i = 0; i < NUM_OBJ_FLAG_BYTES; ++i)
    {
        objSendReqFlags[i] = 0;
    }

    for (uint8_t i = 0; i < commandTableSize(); ++i)
    {
        CmdTab[i].objValues = 0;
    }

    answerWait = 0;
    noAnswerCount = 0;

    alarmBus = 0;
    alarmLocal = 0;

    testAlarmBus = 0;
    testAlarmLocal = 0;

    setAlarmBus = 0;
    setTestAlarmBus = 0;
    ignoreBusAlarm = 0;

    infoSendObjno = 0;
    readCmdno = commandTableSize();
    infoCounter = bcu.userEeprom->getUInt8(CONF_INFO_INTERVAL);
    alarmCounter = 1;
    TalarmCounter = 1;
    delayedAlarmCounter = 0;

    errCode = 0;

    // set all comObjects to default
    for (uint8_t i = 0; i < NUM_OBJS; i++)
    {
        bcu.comObjects->objectSetValue(i, 0);
    }

    pinMode(LED_BASEPLATE_DETACHED_PIN, OUTPUT);
    digitalWrite(LED_BASEPLATE_DETACHED_PIN, false);
    pinMode(LED_SUPPLY_VOLTAGE_DISABLED_PIN, OUTPUT);
    digitalWrite(LED_SUPPLY_VOLTAGE_DISABLED_PIN, false);

    pinMode(RM_ACTIVITY_PIN, INPUT | PULL_DOWN); // smoke detector base plate state, pulldown configured, Pin is connected to 3.3V VCC of the RM
    pinMode(RM_SUPPORT_VOLTAGE_PIN, OUTPUT);
    digitalWrite(RM_SUPPORT_VOLTAGE_PIN, RM_SUPPORT_VOLTAGE_OFF); // zuerst die Spannungsversorgung ausschalten
    delay(SUPPLY_VOLTAGE_OFF_DELAY_MS); ///\todo move to delayed app start, make sure it lasts at least 500ms to discharge the 12V capacitor
}
