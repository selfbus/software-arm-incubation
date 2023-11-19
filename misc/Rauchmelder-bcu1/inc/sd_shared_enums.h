/*
 *  Original written for LPC922:
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
#ifndef SMOKE_DETECTOR_SHARED_ENUMS_H_
#define SMOKE_DETECTOR_SHARED_ENUMS_H_

#include <stdint.h>
#include <type_traits>


/**
 * Smoke detector alarm states
 */
enum class RmAlarmState : uint8_t
{
    noAlarm,   //!< normal state, no alarm and no test alarm
    alarm,     //!< alarm state
    testAlarm, //!< test alarm state
    noChange   //!< do nothing, keep as is
};

//-----------------------------------------------------------------------------
// Communication Objects
//-----------------------------------------------------------------------------
enum class GroupObject : uint8_t
{
    alarmBus =               0, //!< Alarm Network
    testAlarmBus =           1, //!< Test Alarm Network
    resetAlarm =             2, //!< Alarm / Test Alarm Reset
    statusAlarm =            3, //!< Status Alarm
    statusAlarmDelayed =     4, //!< Status Delayed Alarm
    statusTestAlarm =        5, //!< Status Test Alarm
    serialNumber =           6, //!< Serial number
    operatingTime =          7, //!< Operating time in seconds or hours
    smokeboxValue =          8, //!< Smoke box value
    smokeboxPollution =      9, //!< Smoke box pollution
    batteryVoltage =        10, //!< Battery voltage
    temperature =           11, //!< Temperature
    errorCode =             12, //!< Error code, see @ref SdErrorCode
    batteryLow =            13, //!< Battery low
    malfunction =           14, //!< Device defective
    countSmokeAlarm =       15, //!< Number of smoke alarms
    countTemperatureAlarm = 16, //!< Number of temperature alarms
    countTestAlarm =        17, //!< Number of test alarms
    countAlarmWire =        18, //!< Number of wire alarms
    countAlarmBus =         19, //!< Number of bus alarms
    countTestAlarmWire =    20, //!< Number of wire test alarms
    countTestAlarmBus =     21, //!< Number of bus test alarms
};

// https://stackoverflow.com/a/31836401
template < typename T, T beginValue, T endValue >
class EnumIterator
{
    typedef typename std::underlying_type<T>::type underlyingType;

public:
    EnumIterator() : value(static_cast<underlyingType>(beginValue))
    {
    }

    EnumIterator(const T & value) : value(static_cast<underlyingType>(value))
    {
    }

    EnumIterator& operator++()
    {
        ++value;
        return *this;
    }

    T operator*() const
    {
        return static_cast<T>(value);
    }

    bool operator==(const EnumIterator & other) const
    {
        return value == other.value;
    }

    bool operator!=(const EnumIterator & other) const
    {
        return value != other.value;
    }

    EnumIterator begin() const
    {
        return *this;
    }

    EnumIterator end() const
    {
        static const EnumIterator endIterator = ++EnumIterator(endValue);
        return endIterator;
    }

private:
    underlyingType value;
};

typedef EnumIterator<GroupObject, GroupObject::alarmBus, GroupObject::countTestAlarmBus> AllGroupObjects;
typedef EnumIterator<GroupObject, GroupObject::serialNumber, GroupObject::countTestAlarmBus> InfoGroupObjects;

#endif /*SMOKE_DETECTOR_SHARED_ENUMS_H_*/
