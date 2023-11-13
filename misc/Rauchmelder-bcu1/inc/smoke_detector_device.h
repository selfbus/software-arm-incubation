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

#ifndef SMOKE_DETECTOR_DEVICE_H_
#define SMOKE_DETECTOR_DEVICE_H_

#include <stdint.h>
#include <sblib/timeout.h>

#include "smoke_detector_com.h"
#include "rm_const.h"

class SmokeDetectorAlarm;
class SmokeDetectorConfig;
class SmokeDetectorErrorCode;
class SmokeDetectorGroupObjects;

/**
 * Possible commands to send to the smoke detector
 */
enum class DeviceCommand : uint8_t
{
    serialNumber          = 0,  //!< Gira Command: Read serial number
    operatingTime         = 1,  //!< Gira Command: Query operating time
    smokeboxData          = 2,  //!< Gira Command: Query smokebox data
    batteryAndTemperature = 3,  //!< Gira Command: Query battery voltage and temperatures
    numberAlarms1         = 4,  //!< Gira Command: Query number of alarms #1
    numberAlarms2         = 5,  //!< Gira Command: Query number of alarms #2
    status                = 6,  //!< Gira Command: Query status
};

typedef EnumIterator<DeviceCommand, DeviceCommand::serialNumber, DeviceCommand::status> AllDeviceCommands;

class SmokeDetectorDevice : private SmokeDetectorComCallback
{
public:
    SmokeDetectorDevice(const SmokeDetectorConfig *config, const SmokeDetectorGroupObjects *groupObjects, SmokeDetectorAlarm *alarm, SmokeDetectorErrorCode *errorCode);
    ~SmokeDetectorDevice() = delete;

    void setAlarmState(RmAlarmState newState);
    bool sendCommand(DeviceCommand cmd);
    void receiveBytes();
    void timerEvery500ms();
    void checkState();

private:
    void receivedMessage(uint8_t *bytes, int8_t len);

private:
    bool hasOngoingMessageExchange() const;
    void setSupplyVoltage(bool enable);
    void checkAttachedToBasePlate();

    void readSerialNumberMessage(const uint8_t *bytes) const;
    void readOperatingTimeMessage(const uint8_t *bytes) const;
    void readSmokeboxDataMessage(const uint8_t *bytes) const;
    void readBatteryAndTemperatureMessage(const uint8_t *bytes) const;
    void readNumberAlarms1Message(const uint8_t *bytes) const;
    void readNumberAlarms2Message(const uint8_t *bytes) const;
    void readStatusMessage(const uint8_t *bytes) const;
    uint32_t readOperatingTime(const uint8_t *bytes) const;
    uint32_t readVoltage(const uint8_t *bytes) const;
    uint32_t readTemperature(const uint8_t *bytes) const;

    static void failHardInDebug();
    static RmCommandByte deviceCommandToRmCommandByte(DeviceCommand command);
    static uint32_t readUInt32(const uint8_t *bytes);
    static uint16_t readUInt16(const uint8_t *bytes);

private:
    enum class DeviceState
    {
        drainCapacitor,
        attachToBasePlate,
        powerUpDevice,
        fillCapacitor,
        running
    };

    //!< Time in milliseconds the 12V supply needs to drain the capacitor
    static constexpr int SupplyVoltageOffDelayMs = 500;

    //!< Time in milliseconds the 12V supply needs to fill the capacitor
    static constexpr int SupplyVoltageOnDelayMs = 5000;

    //!< Maximum waiting time in milliseconds to enable 12V supply
    static constexpr int SupplyVoltageTimeoutMs = 120000;

    //!< Time in milliseconds we give the smoke detector to startup and measure the battery voltage
    static constexpr int DevicePowerUpDelayMs = 1000;

private:
    const SmokeDetectorConfig *config;
    const SmokeDetectorGroupObjects *groupObjects;
    SmokeDetectorAlarm *alarm;
    SmokeDetectorErrorCode *errorCode;
    SmokeDetectorCom *com;
    DeviceState state;
    Timeout timeout;
    unsigned char answerWait;          //!< Wenn != 0, dann Zähler für die Zeit die auf eine Antwort vom Rauchmelder gewartet wird.
};

#endif /* SMOKE_DETECTOR_DEVICE_H_ */
