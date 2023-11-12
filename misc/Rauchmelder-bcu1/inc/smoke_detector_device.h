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

private:
    void receivedMessage(uint8_t *bytes, int8_t len);

private:
    void failHardInDebug();
    static uint8_t commandTableSize();
    bool hasOngoingMessageExchange();

    /**
     * Read raw value from response and convert it to corresponding group object value
     */
    uint32_t readObjectValueFromResponse(uint8_t *rawValue, uint8_t dataType);

    void setSupplyVoltageAndWait(bool enable, uint32_t waitTimeMs);
    void checkRmAttached2BasePlate();

private:
    const SmokeDetectorConfig *config;
    const SmokeDetectorGroupObjects *groupObjects;
    SmokeDetectorAlarm *alarm;
    SmokeDetectorErrorCode *errorCode;
    SmokeDetectorCom *com;
    unsigned char answerWait;          //!< Wenn != 0, dann Zähler für die Zeit die auf eine Antwort vom Rauchmelder gewartet wird.
};

#endif /* SMOKE_DETECTOR_DEVICE_H_ */
