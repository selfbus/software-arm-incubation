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

#include <sblib/eib/datapoint_types.h>
#include <sblib/digital_pin.h>
#include <sblib/timer.h>
#include <sblib/utils.h>

#include "smoke_detector_com.h"
#include "rm_const.h"
#include "smoke_detector_alarm.h"
#include "smoke_detector_config.h"
#include "smoke_detector_device.h"
#include "smoke_detector_errorcode.h"
#include "smoke_detector_group_objects.h"

#define INITIAL_ANSWER_WAIT 6      //!< Initialwert für answerWait in 0,5s

SmokeDetectorDevice::SmokeDetectorDevice(const SmokeDetectorConfig *config, const SmokeDetectorGroupObjects *groupObjects, SmokeDetectorAlarm *alarm, SmokeDetectorErrorCode *errorCode)
    : config(config),
      groupObjects(groupObjects),
      alarm(alarm),
      errorCode(errorCode),
      com(new SmokeDetectorCom(this))
{
    answerWait = 0;

    pinMode(LED_BASEPLATE_DETACHED_PIN, OUTPUT);
    digitalWrite(LED_BASEPLATE_DETACHED_PIN, false);
    pinMode(RM_ACTIVITY_PIN, INPUT | PULL_DOWN); // smoke detector base plate state, pulldown configured, Pin is connected to 3.3V VCC of the RM

    setSupplyVoltageAndWait(false, SUPPLY_VOLTAGE_OFF_DELAY_MS);  ///\todo move waiting to delayed app start, make sure it lasts at least 500ms to discharge the 12V capacitor
}

/**
 * @ref RmCommandByte to multiple @ref GroupObject association table
 */
constexpr struct
{
    RmCommandByte rmCommand;      //!< @ref RmCommandByte command to be sent
    int8_t responseLength;        //!< Expected response length in bytes
    struct
    {
        GroupObject object;       //!< @ref GroupObject in the response
        uint8_t offset;           //!< Byte-offset in the response
        uint8_t dataType;         //!< Datatype of the object in the response
    } objects[MAX_OBJ_CMD];       //!< Association of the command to multiple @ref GroupObject. Use @ref grpObjInvalid to not associate
}
CmdTab[] =
{
    // CommandByte                  Response length         Object number   Raw value  Example response
    {RmCommandByte::serialNumber,           5, {{GroupObject::serialNumber,          0, RM_TYPE_LONG},      // <STX>C4214710F31F<ETX>
                                                {GroupObject::none},
                                                {GroupObject::none},
                                                {GroupObject::none}}},
    {RmCommandByte::operatingTime,          5, {{GroupObject::operatingTime,         0, RM_TYPE_QSEC},      // <STX>C9000047E31F<ETX>
                                                {GroupObject::none},
                                                {GroupObject::none},
                                                {GroupObject::none}}},
    {RmCommandByte::smokeboxData,           5, {{GroupObject::smokeboxValue,         0, RM_TYPE_SHORT},     // <STX>CB0065000111<ETX>
                                                {GroupObject::smokeboxPollution,     3, RM_TYPE_BYTE},
                                                {GroupObject::countSmokeAlarm,       2, RM_TYPE_BYTE},
                                                {GroupObject::none}}},
    {RmCommandByte::batteryTemperatureData, 5, {{GroupObject::batteryVoltage,        0, RM_TYPE_MVOLT},     // <STX>CC000155551B<ETX>
                                                {GroupObject::temperature,           2, RM_TYPE_TEMP},
                                                {GroupObject::none},
                                                {GroupObject::none}}},
    {RmCommandByte::numberAlarms_1,         5, {{GroupObject::countTemperatureAlarm, 0, RM_TYPE_BYTE},      // <STX>CD0000000007<ETX>
                                                {GroupObject::countTestAlarm,        1, RM_TYPE_BYTE},
                                                {GroupObject::countAlarmWire,        2, RM_TYPE_BYTE},
                                                {GroupObject::countAlarmBus,         3, RM_TYPE_BYTE}}},
    {RmCommandByte::numberAlarms_2,         3, {{GroupObject::countTestAlarmWire,    0, RM_TYPE_BYTE},      // <STX>CE000048<ETX>
                                                {GroupObject::countTestAlarmBus,     1, RM_TYPE_BYTE},
                                                {GroupObject::none},
                                                {GroupObject::none}}},
    {RmCommandByte::status,                 5, {{GroupObject::none},                                        // <STX>C220000000F7<ETX>
                                                {GroupObject::none},
                                                {GroupObject::none},
                                                {GroupObject::none}}}
};

void SmokeDetectorDevice::failHardInDebug() ///\todo remove on release
{
#ifdef DEBUG
    fatalError();
#endif
}

uint8_t SmokeDetectorDevice::commandTableSize()
{
    return sizeof(CmdTab)/sizeof(CmdTab[0]);
}

bool SmokeDetectorDevice::hasOngoingMessageExchange()
{
    return answerWait != 0;
}

void SmokeDetectorDevice::timerEvery500ms()
{
    // Wir warten auf eine Antwort vom Rauchmelder
    if (answerWait)
    {
        --answerWait;
        if (!answerWait)
        {
            errorCode->communicationTimeout(true);
        }
    }
}

/**
 * For description see declaration in file @ref smoke_detector_com.h
 */
void SmokeDetectorDevice::receivedMessage(uint8_t *bytes, int8_t len)
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
        // Informationen aus den empfangenen Daten vom Rauchmelder der sblib zur Verfügung stellen
        // Dazu alle Com-Objekte suchen auf die die empfangenen Daten passen (mapping durch CmdTab)
        // notwendig für den Abruf von Informationen über KNX aus den Status Objekten (GroupValueRead -> GroupValueResponse)
        for (unsigned char cmdObj_cnt = 0; (CmdTab[cmd].objects[cmdObj_cnt].object != GroupObject::none) &&
                                           (cmdObj_cnt < MAX_OBJ_CMD); cmdObj_cnt++)
        {
            auto groupObject = CmdTab[cmd].objects[cmdObj_cnt].object;
            auto offset = CmdTab[cmd].objects[cmdObj_cnt].offset;
            auto dataType = CmdTab[cmd].objects[cmdObj_cnt].dataType;
            auto value = readObjectValueFromResponse(bytes + 1 + offset, dataType);
            groupObjects->setValue(groupObject, value);
        }
    }
    else // status command gets special treatment
    {
        unsigned char subType = bytes[1];

        // (Alarm) Status

        unsigned char status = bytes[2];

        // Lokaler Alarm: Rauch Alarm | Temperatur Alarm | Wired Alarm
        auto hasAlarm = (subType & 0x10) | (status & (0x04 | 0x08));

        // Lokaler Testalarm: (lokaler) Testalarm || Wired Testalarm
        auto hasTestAlarm = status & (0x20 | 0x40);

        auto hasAlarmFromBus = status & 0x10;
        auto hasTestAlarmFromBus = status & 0x80;

        alarm->deviceStatusUpdate(hasAlarm, hasTestAlarm, hasAlarmFromBus, hasTestAlarmFromBus);

        if (subType & 0x08)  // Taste am Rauchmelder gedrückt
        {
            alarm->deviceButtonPressed();
        }

        // Battery low
        bool batteryLow = ((status & 0x01) == 1);
        errorCode->batteryLow(batteryLow);

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
 * Read raw value from response and convert it to corresponding group object value.
 *
 * @param rawValue Pointer to the raw value in the smoke detector response message
 * @param dataType Data type of the value
 * @return Group object value
 */
uint32_t SmokeDetectorDevice::readObjectValueFromResponse(uint8_t *rawValue, uint8_t dataType)
{
    uint32_t lval;

    switch (dataType)
    {
        case RM_TYPE_BYTE:
            return *rawValue;

        case RM_TYPE_LONG:
            return answer_to_long(rawValue);

        case RM_TYPE_QSEC:  // Betriebszeit verarbeiten
            lval = answer_to_long(rawValue) >> 2; // Wert in Sekunden
            if (config->infoSendOperationTimeInHours())
                return lval / 3600; // Stunden, 16Bit
            else
                return lval;        // Sekunden, 32Bit

        case RM_TYPE_SHORT:
            return answer_to_short(rawValue);

        case RM_TYPE_TEMP:
            // Conversion per temp sensor: (answer[x] * 0.5°C - 20°C) * 100 [for DPT9]
            lval = ((int) rawValue[0]) + rawValue[1];
            lval *= 25; // in lval sind zwei Temperaturen, daher halber Multiplikator
            lval -= 2000;
            lval += config->temperatureOffsetInTenthDegrees() * 10;  // Temperaturabgleich
            return (floatToDpt9(lval));

        case RM_TYPE_MVOLT:
            if ((rawValue[0] == 0) && (rawValue[1] == 1))
            {
                return (floatToDpt9(BATTERY_VOLTAGE_INVALID));
            }
            lval = answer_to_short(rawValue);
            // Conversion: lval * 5.7 * 3.3V / 1024 * 1000mV/V * 100 [for DPT9]
            lval *= 9184;
            lval /= 5;
            return (floatToDpt9(lval));

        default: // Fehler: unbekannter Datentyp
            return -2;
    }
}

/**
 * Enable/disable the 12V supply voltage
 *
 * @param enable     Set true to enable supply voltage, false to disable it
 * @param waitTimeMs Time in milliseconds to wait after supply voltage was enabled/disabled
 */
void SmokeDetectorDevice::setSupplyVoltageAndWait(bool enable, uint32_t waitTimeMs)
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
void SmokeDetectorDevice::checkRmAttached2BasePlate()
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
        com->initSerialCom(); //serielle Schnittstelle für die Kommunikation mit dem Rauchmelder initialisieren
    }
}

/**
 * Send command @ref cmd to smoke detector.\n
 * Receiving and processing the response from the smoke detector is done in @ref receivedMessage().
 *
 * @param cmd - Index of the command to be send from the @ref CmdTab
 * @return True if command was sent, otherwise false.
 */
bool SmokeDetectorDevice::send_Cmd(DeviceCommand cmd)
{
    checkRmAttached2BasePlate(); ///\todo If think this should be moved to TIMER32_0_IRQHandler

    if (com->isReceiving())
    {
        return false;
    }

    if (!com->sendCommand(CmdTab[(uint8_t)cmd].rmCommand))
    {
        return false;
    }

    ///\todo setting group obj values to invalid, should be done after a serial timeout occurred
    switch (cmd)
    {
        case DeviceCommand::serialNumber:
            break;

        case DeviceCommand::operatingTime:
            break;

        case DeviceCommand::smokeboxData:
            break;

        case DeviceCommand::batteryAndTemperature:
            groupObjects->setValue(GroupObject::batteryVoltage, 0);
            groupObjects->setValue(GroupObject::temperature, 0);
            break;

        case DeviceCommand::numberAlarms1:
            break;

        case DeviceCommand::numberAlarms2:
            break;

        default:
            break;
    }

    answerWait = INITIAL_ANSWER_WAIT;
    return true;
}

void SmokeDetectorDevice::recv_bytes()
{
    com->receiveBytes();
}

void SmokeDetectorDevice::set_alarm_state(RmAlarmState newState)
{
    com->setAlarmState(newState);
    answerWait = INITIAL_ANSWER_WAIT;
}
