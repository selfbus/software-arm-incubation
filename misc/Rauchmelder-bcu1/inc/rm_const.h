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
#ifndef rm_const_h
#define rm_const_h

#include <stdint.h>
#include <type_traits>
#include <sblib/io_pin_names.h>


// IO Pin assignments
#define RM_COMM_ENABLE_PIN    PIO3_5
#define RM_COMM_ENABLE false //!< set low to enable smoke detector's serial communication feature

#define RM_ACTIVITY_PIN PIO0_11 //!< IO Pin an dem die Erkennung der internen Rauchmelder Spannung angeschlossen ist
#define RM_IS_ACTIVE true

#define RM_SUPPORT_VOLTAGE_PIN PIO2_1 //!< Ansteuerung der Spannungsversorgung zur Untertützung des Rauchmelders
#define RM_SUPPORT_VOLTAGE_ON false
#define RM_SUPPORT_VOLTAGE_OFF true //!< NPN Transistor zieht die Versorgungsspannung herunter

// LED pins
#define LED_BASEPLATE_DETACHED_PIN PIO2_6       //!< LED is on, while base plate is not attached means @ref RM_ACTIVITY_PIN is false / off
#define LED_SUPPLY_VOLTAGE_DISABLED_PIN PIO3_3  //!< LED is on, while supply voltage is "disconnected" means @ref RM_SUPPORT_VOLTAGE_PIN is true

// Sentinel values
#define BATTERY_VOLTAGE_INVALID -1              //!< Value representing an invalid battery voltage

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
// Kommunikations Objekte
//-----------------------------------------------------------------------------
enum class GroupObject : uint8_t
{
    alarmBus =               0, //!< Alarm Vernetzung
    testAlarmBus =           1, //!< Testalarm Vernetzung
    resetAlarm =             2, //!< Alarm / Testalarm rücksetzen
    statusAlarm =            3, //!< Status Alarm
    statusAlarmDelayed =     4, //!< Status verzögerter Alarm
    statusTestAlarm =        5, //!< Status Testalarm
    serialNumber =           6, //!< Rauchmelder Seriennummer
    operatingTime =          7, //!< Rauchmelder Betriebsdauer in Sekunden
    smokeboxValue =          8, //!< Rauchkammer Wert
    smokeboxPollution =      9, //!< Rauchkammer Verschmutzung
    batteryVoltage =        10, //!< Batterie Spannung
    temperature =           11, //!< Temperatur
    errorCode =             12, //!< Rauchmelder Fehlercode, siehe @ref SdErrorCode
    batteryLow =            13, //!< Batterie leer
    malfunction =           14, //!< Rauchmelder defekt
    countSmokeAlarm =       15, //!< Anzahl Rauchalarme
    countTemperatureAlarm = 16, //!< Anzahl Temperatur Alarme
    countTestAlarm =        17, //!< Anzahl Testalarme
    countAlarmWire =        18, //!< Anzahl Draht Alarme
    countAlarmBus =         19, //!< Anzahl Bus Alarme
    countTestAlarmWire =    20, //!< Anzahl Draht Testalarme
    countTestAlarmBus =     21, //!< Anzahl Bus Testalarme

    none =                 0xff //!< No group object
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

    EnumIterator operator++()
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

#endif /*rm_const_h*/
