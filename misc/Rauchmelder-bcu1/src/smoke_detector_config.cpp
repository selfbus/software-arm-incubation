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

#include <sblib/eib/memory.h>

#include "smoke_detector_config.h"

enum ConfigAddress : uint32_t
{
    SendEnable = 507,         //!< Enable/disable periodic and delayed sending
    AlarmInterval = 506,      //!< Interval for periodic sending of alarm, in seconds
    AlarmDelayed = 504,       //!< Delay for delayed sending of alarm, in seconds
    TestAlarmInterval = 503,  //!< Interval for periodic sending of test alarm, in seconds
    InfoInterval = 505,       //!< Interval for periodic sending of status information, in minutes
    InfoEnabledObjects = 508, //!< Enable/disable periodic sending of status information group objects 6..21 (2 bytes)
    TemperatureOffset = 510   //!< Temperature calibration, -12..+12 degree in steps of 0.1 degrees
};

enum SendEnable
{
    PeriodicAlarmStatusWhenNoAlarm = 0x01, //!< Periodic sending of alarm status when there is no alarm
    PeriodicTestAlarmNetwork = 0x02,       //!< Periodic sending of test alarm network
    PeriodicAlarmNetwork = 0x04,           //!< Periodic sending of alarm network
    OperationTimeInHours = 0x08,           //!< Send operation time in hours
    PeriodicInfo = 0x10,                   //!< Periodic sending of status information
    PeriodicTestAlarmStatus = 0x20,        //!< Periodic sending of test alarm status
    PeriodicAlarmStatus = 0x40,            //!< Periodic sending of alarm status
    DelayedAlarm = 0x80                    //!< Delayed sending of alarm
};

SmokeDetectorConfig::SmokeDetectorConfig(Memory *memory)
    : memory(memory)
{
}

//-----------------------------------------------------------------------------
// Alarm page
//-----------------------------------------------------------------------------

bool SmokeDetectorConfig::alarmSendStatusPeriodically() const
{
    return memory->getUInt8(ConfigAddress::SendEnable) & SendEnable::PeriodicAlarmStatus;
}

bool SmokeDetectorConfig::alarmSendStatusPeriodicallyWhenNoAlarm() const
{
    return memory->getUInt8(ConfigAddress::SendEnable) & SendEnable::PeriodicAlarmStatusWhenNoAlarm;
}

bool SmokeDetectorConfig::alarmSendNetworkPeriodically() const
{
    return memory->getUInt8(ConfigAddress::SendEnable) & SendEnable::PeriodicAlarmNetwork;
}

uint8_t SmokeDetectorConfig::alarmIntervalSeconds() const
{
    return memory->getUInt8(ConfigAddress::AlarmInterval);
}

bool SmokeDetectorConfig::alarmSendDelayed() const
{
    return memory->getUInt8(ConfigAddress::SendEnable) & SendEnable::DelayedAlarm;
}

uint8_t SmokeDetectorConfig::alarmDelaySeconds() const
{
    return memory->getUInt8(ConfigAddress::AlarmDelayed);
}

//-----------------------------------------------------------------------------
// Test alarm page
//-----------------------------------------------------------------------------

bool SmokeDetectorConfig::testAlarmSendStatusPeriodically() const
{
    return memory->getUInt8(ConfigAddress::SendEnable) & SendEnable::PeriodicTestAlarmStatus;
}

bool SmokeDetectorConfig::testAlarmSendNetworkPeriodically() const
{
    return memory->getUInt8(ConfigAddress::SendEnable) & SendEnable::PeriodicTestAlarmNetwork;
}

uint8_t SmokeDetectorConfig::testAlarmIntervalSeconds() const
{
    return memory->getUInt8(ConfigAddress::TestAlarmInterval);
}

//-----------------------------------------------------------------------------
// Status informations page
//-----------------------------------------------------------------------------

bool SmokeDetectorConfig::infoSendAnyPeriodically() const
{
    return memory->getUInt8(ConfigAddress::SendEnable) & SendEnable::PeriodicInfo;
}

uint8_t SmokeDetectorConfig::infoIntervalMinutes() const
{
    return memory->getUInt8(ConfigAddress::InfoInterval);
}

bool SmokeDetectorConfig::infoSendPeriodically(GroupObject groupObject) const
{
    auto objectNumber = static_cast<uint8_t>(groupObject);

    if (objectNumber < 6 || objectNumber > 21)
        return false;

    // Bits are stored in ascending memory address and object number order:
    //
    // Byte           |            508            |            509
    // Object Number  |   6  7  8  9 10 11 12 13  |  14 15 16 17 18 19 20 21
    // Hex value      |  80 40 20 10  8  4  2  1  |  80 40 20 10  8  4  2  1

    return memory->getUInt16(ConfigAddress::InfoEnabledObjects) & (1 << (21 - objectNumber));
}

bool SmokeDetectorConfig::infoSendOperationTimeInHours() const
{
    return memory->getUInt8(ConfigAddress::SendEnable) & SendEnable::OperationTimeInHours;
}

//-----------------------------------------------------------------------------
// Temperature page
//-----------------------------------------------------------------------------

int8_t SmokeDetectorConfig::temperatureOffsetInTenthDegrees() const
{
    return static_cast<int8_t>(memory->getUInt8(ConfigAddress::TemperatureOffset));
}
