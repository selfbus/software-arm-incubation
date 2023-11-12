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

// Time-related values
#define SUPPLY_VOLTAGE_ON_DELAY_MS  5000        //!< Time in milliseconds the 12V supply needs to raise 
#define SUPPLY_VOLTAGE_OFF_DELAY_MS  500        //!< Time in milliseconds the 12V supply needs to drain the capacitor
#define SUPPLY_VOLTAGE_TIMEOUT_MS 120000        //!< Maximum waiting time in milliseconds to enable 12V supply
#define RM_POWER_UP_TIME_MS         1000        //!< Time in milliseconds we give the smoke detector to startup and measure the battery voltage

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
template < GroupObject beginValue, GroupObject endValue>
class GroupObjectIterator
{
public:
    GroupObjectIterator() : value(static_cast<uint8_t>(beginValue))
    {
    }

    GroupObjectIterator(const GroupObject & groupObject) : value(static_cast<uint8_t>(groupObject))
    {
    }

    GroupObjectIterator operator++()
    {
        ++value;
        return *this;
    }

    GroupObject operator*() const
    {
        return static_cast<GroupObject>(value);
    }

    bool operator!=(const GroupObjectIterator & groupObject) const
    {
        return value != groupObject.value;
    }

    GroupObjectIterator begin() const
    {
        return *this;
    }

    GroupObjectIterator end() const
    {
        static const GroupObjectIterator endIterator = ++GroupObjectIterator(endValue);
        return endIterator;
    }

private:
    uint8_t value;
};

typedef GroupObjectIterator<GroupObject::alarmBus, GroupObject::countTestAlarmBus> AllGroupObjects;
typedef GroupObjectIterator<GroupObject::serialNumber, GroupObject::countTestAlarmBus> InfoGroupObjects;

//!< Maximale Anzahl von ComObjekten pro Rauchmelder-Befehl
#define MAX_OBJ_CMD  4

//-----------------------------------------------------------------------------
// Datentypen der Rauchmelder Antworten
//-----------------------------------------------------------------------------

#define RM_TYPE_LONG   1 //!< Rauchmelder Antwort: 4 Byte Zahl
#define RM_TYPE_SHORT  2 //!< Rauchmelder Antwort: 2 Byte Zahl
#define RM_TYPE_BYTE   3 //!< Rauchmelder Antwort: 1 Byte Zahl
#define RM_TYPE_TEMP   4 //!< Rauchmelder Antwort: 1 Byte Temperatur
#define RM_TYPE_MVOLT  5 //!< Rauchmelder Antwort: 2 Byte Spannung
#define RM_TYPE_QSEC   6 //!< Rauchmelder Antwort: 4 Byte 1/4 Sekunden
#define RM_TYPE_NONE 127 //!< Rauchmelder Antwort: Kein Typ

//-----------------------------------------------------------------------------
// Kommunikations Konstanten
//-----------------------------------------------------------------------------
#define NUL         0x00 //!> Null byte (prefix of @ref STX start byte)
#define STX         0x02 //!> Start byte
#define ETX         0x03 //!> End byte
#define ACK         0x06 //!> Acknowledged byte
#define NAK         0x15 //!< Not acknowledged byte

#endif /*rm_const_h*/
