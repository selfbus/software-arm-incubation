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

#include <type_traits>
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

void SmokeDetectorDevice::setAlarmState(RmAlarmState newState)
{
    // While waiting for an answer we don't process alarms to avoid overlapping message exchanges.
    // As a message exchange is fast and this is called from the main loop that's fine.
    if (hasOngoingMessageExchange())
    {
       return;
    }

    com->setAlarmState(newState);
    answerWait = INITIAL_ANSWER_WAIT;
}

/**
 * Send command @ref cmd to smoke detector.\n
 * Receiving and processing the response from the smoke detector is done in @ref receivedMessage().
 *
 * @param cmd - Command to send
 * @return True if command was sent, otherwise false.
 */
bool SmokeDetectorDevice::sendCommand(DeviceCommand cmd)
{
    checkRmAttached2BasePlate(); ///\todo If think this should be moved to TIMER32_0_IRQHandler

    if (hasOngoingMessageExchange())
    {
        return false;
    }

    if (!com->sendCommand(deviceCommandToRmCommandByte(cmd)))
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

void SmokeDetectorDevice::receiveBytes()
{
    com->receiveBytes();
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
    uint8_t msgType;

    answerWait = 0;
    errorCode->communicationTimeout(false);

    msgType = bytes[0];
    const auto rmCommandByteStatus = static_cast<std::underlying_type_t<RmCommandByte>>(RmCommandByte::status);
    if (msgType == (rmCommandByteStatus | 0x80))
    {
        msgType = (rmCommandByteStatus | 0xc0); // "cast" automatic status message to "normal" status message
    }

    if ((msgType & 0xf0) != 0xc0) // check for valid response byte
    {
                // receiving an "echo" of our own command (e.g. <STX>0262<ETX>) can bring us here.
        return; // learned this by accidently touching the ARM's tx/rx pins
    }

    auto rmCommandByte = static_cast<RmCommandByte>(msgType & 0x0f);

    // Received length must match expected length.
    auto expectedLength = (rmCommandByte == RmCommandByte::numberAlarms_2) ? 3 : 5;
    if (len != expectedLength)
    {
        failHardInDebug();
        return;
    }

    ++bytes;
    switch (rmCommandByte)
    {
        case RmCommandByte::serialNumber:
            readSerialNumberMessage(bytes);
            break;

        case RmCommandByte::operatingTime:
            readOperatingTimeMessage(bytes);
            break;

        case RmCommandByte::smokeboxData:
            readSmokeboxDataMessage(bytes);
            break;

        case RmCommandByte::batteryTemperatureData:
            readBatteryAndTemperatureMessage(bytes);
            break;

        case RmCommandByte::numberAlarms_1:
            readNumberAlarms1Message(bytes);
            break;

        case RmCommandByte::numberAlarms_2:
            readNumberAlarms2Message(bytes);
            break;

        case RmCommandByte::status:
            readStatusMessage(bytes);
            break;

        default:
            failHardInDebug(); // found a new command/response => time to implement it
            break;
    }
}

bool SmokeDetectorDevice::hasOngoingMessageExchange() const
{
    return answerWait != 0;
}

void SmokeDetectorDevice::readSerialNumberMessage(const uint8_t *bytes) const
{
    // <STX>C4214710F31F<ETX>
    groupObjects->setValue(GroupObject::serialNumber, readUInt32(bytes));
}

void SmokeDetectorDevice::readOperatingTimeMessage(const uint8_t *bytes) const
{
    // <STX>C9000047E31F<ETX>
    // Raw value is in quarter seconds, so divide by 4.
    auto seconds = readUInt32(bytes) >> 2;
    auto value = config->infoSendOperationTimeInHours() ? seconds / 3600 : seconds;
    groupObjects->setValue(GroupObject::operatingTime, value);
}

void SmokeDetectorDevice::readSmokeboxDataMessage(const uint8_t *bytes) const
{
    // <STX>CB0065000111<ETX>
    groupObjects->setValue(GroupObject::smokeboxValue, readUInt16(bytes));
    groupObjects->setValue(GroupObject::countSmokeAlarm, bytes[2]);
    groupObjects->setValue(GroupObject::smokeboxPollution, bytes[3]);
}

void SmokeDetectorDevice::readBatteryAndTemperatureMessage(const uint8_t *bytes) const
{
    // <STX>CC000155551B<ETX>
    uint32_t voltage;
    uint32_t temperature;

    if ((bytes[0] == 0) && (bytes[1] == 1))
    {
        voltage = BATTERY_VOLTAGE_INVALID;
    }
    else
    {
        auto rawVoltage = static_cast<uint32_t>(readUInt16(bytes));
        // Conversion: lval * 5.7 * 3.3V / 1024 * 1000mV/V * 100 [for DPT9]
        voltage = (rawVoltage * 9184) / 5;
    }

    // Conversion per temp sensor: (answer[x] * 0.5°C - 20°C) * 100 [for DPT9]
    temperature = static_cast<uint32_t>(bytes[2]) + bytes[3];
    temperature *= 25; // Added two temperatures, so only half the multiplier
    temperature -= 2000;
    temperature += config->temperatureOffsetInTenthDegrees() * 10;  // Temperature offset

    groupObjects->setValue(GroupObject::batteryVoltage, floatToDpt9(voltage));
    groupObjects->setValue(GroupObject::temperature, floatToDpt9(temperature));
}

void SmokeDetectorDevice::readNumberAlarms1Message(const uint8_t *bytes) const
{
    // <STX>CD0000000007<ETX>
    groupObjects->setValue(GroupObject::countTemperatureAlarm, bytes[0]);
    groupObjects->setValue(GroupObject::countTestAlarm, bytes[1]);
    groupObjects->setValue(GroupObject::countAlarmWire, bytes[2]);
    groupObjects->setValue(GroupObject::countAlarmBus, bytes[3]);
}

void SmokeDetectorDevice::readNumberAlarms2Message(const uint8_t *bytes) const
{
    // <STX>CE000048<ETX>
    groupObjects->setValue(GroupObject::countTestAlarmWire, bytes[0]);
    groupObjects->setValue(GroupObject::countTestAlarmBus, bytes[1]);
}

void SmokeDetectorDevice::readStatusMessage(const uint8_t *bytes) const
{
    // <STX>C220000000F7<ETX>
    unsigned char subType = bytes[0];

    // (Alarm) Status

    unsigned char status = bytes[1];

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
        temperatureSensor_1_fault = (bytes[3] & 0x04); // sensor 1 state in 4. byte 2. bit
        temperatureSensor_2_fault = (bytes[3] & 0x10); // sensor 2 state in 4. byte 4. bit
    }

    errorCode->temperature_1_fault(temperatureSensor_1_fault);
    errorCode->temperature_2_fault(temperatureSensor_2_fault);

    ///\todo handle smoke box fault
    ///
}

void SmokeDetectorDevice::failHardInDebug() ///\todo remove on release
{
#ifdef DEBUG
    fatalError();
#endif
}

RmCommandByte SmokeDetectorDevice::deviceCommandToRmCommandByte(DeviceCommand command)
{
    switch (command)
    {
        case DeviceCommand::serialNumber:
            return RmCommandByte::serialNumber;

        case DeviceCommand::status:
            return RmCommandByte::status;

        case DeviceCommand::batteryAndTemperature:
            return RmCommandByte::batteryTemperatureData;

        case DeviceCommand::operatingTime:
            return RmCommandByte::operatingTime;

        case DeviceCommand::smokeboxData:
            return RmCommandByte::smokeboxData;

        case DeviceCommand::numberAlarms1:
            return RmCommandByte::numberAlarms_1;

        case DeviceCommand::numberAlarms2:
            return RmCommandByte::numberAlarms_2;

        default:
            failHardInDebug();
            return RmCommandByte::status;
    }
}

/**
 * Read a 32-bit unsigned number from the smoke detector.
 *
 * @param bytes First byte of the smoke detector response.
 * @return The value in processor native format.
 */
uint32_t SmokeDetectorDevice::readUInt32(const uint8_t *bytes)
{
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

/**
 * Read a 16-bit unsigned number from the smoke detector.
 *
 * @param bytes First byte of the smoke detector response.
 * @return The value in processor native format.
 */
uint16_t SmokeDetectorDevice::readUInt16(const uint8_t *bytes)
{
    return (bytes[0] << 8) | bytes[1];
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
