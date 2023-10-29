/*
 *  Original written for LPC922:
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  Modified for LPC1115 ARM processor:
 *  Copyright (c) 2017 Oliver Stefan <o.stefan252@googlemail.com>
 *  Copyright (c) 2020 Stefan Haller
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
#define TIMER_INTERVAL_MS            500        //!< Periodic timer interval in milliseconds (handles all periodic tasks)
#define SUPPLY_VOLTAGE_ON_DELAY_MS  5000        //!< Time in milliseconds the 12V supply needs to raise 
#define SUPPLY_VOLTAGE_OFF_DELAY_MS  500        //!< Time in milliseconds the 12V supply needs to drain the capacitor
#define SUPPLY_VOLTAGE_TIMEOUT_MS 120000        //!< Maximum waiting time in milliseconds to enable 12V supply
#define RM_POWER_UP_TIME_MS         1000        //!< Time in milliseconds we give the smoke detector to startup and measure the battery voltage

// counters are in half seconds (1=0.5s, 120=60.0s)
#ifdef DEBUG
    #define DEFAULT_EVENTTIME 60
    #define DEFAULT_SERIAL_COMMAND_TIME 7
    #define DEFAULT_KNX_OBJECT_TIME 1
#else
    #define DEFAULT_EVENTTIME 120  // Initialisierung auf 1 Minute (sonst wird im Timer Interrupt 0 minus 1 durchgeführt)
    #define DEFAULT_SERIAL_COMMAND_TIME 15 // half seconds
    #define DEFAULT_KNX_OBJECT_TIME 3 // half seconds
#endif

//-----------------------------------------------------------------------------
// Kommunikations Objekte
//-----------------------------------------------------------------------------
enum GroupObject : uint8_t
{
    grpObjAlarmBus =               0, //!< Alarm Vernetzung
    grpObjTestAlarmBus =           1, //!< Testalarm Vernetzung
    grpObjResetAlarm =             2, //!< Alarm / Testalarm rücksetzen
    grpObjStatusAlarm =            3, //!< Status Alarm
    grpObjStatusAlarmDelayed =     4, //!< Status verzögerter Alarm
    grpObjStatusTestAlarm =        5, //!< Status Testalarm
    grpObjSerialNumber =           6, //!< Rauchmelder Seriennummer
    grpObjOperatingTime =          7, //!< Rauchmelder Betriebsdauer in Sekunden
    grpObjSmokeboxValue =          8, //!< Rauchkammer Wert
    grpObjSmokeboxPollution =      9, //!< Rauchkammer Verschmutzung
    grpObjBatteryVoltage =        10, //!< Batterie Spannung
    grpObjTemperature =           11, //!< Temperatur
    grpObjErrorCode =             12, //!< Rauchmelder Fehlercode, siehe @ref SdErrorCode
    grpObjBatteryLow =            13, //!< Batterie leer
    grpObjMalfunction =           14, //!< Rauchmelder defekt
    grpObjCountSmokeAlarm =       15, //!< Anzahl Rauchalarme
    grpObjCountTemperatureAlarm = 16, //!< Anzahl Temperatur Alarme
    grpObjCountTestAlarm =        17, //!< Anzahl Testalarme
    grpObjCountAlarmWire =        18, //!< Anzahl Draht Alarme
    grpObjCountAlarmBus =         19, //!< Anzahl Bus Alarme
    grpObjCountTestAlarmWire =    20, //!< Anzahl Draht Testalarme
    grpObjCountTestAlarmBus =     21, //!< Anzahl Bus Testalarme

    grpObjInvalid =              0xff //!< Invalid groupobject
};

#define NUM_OBJS               22 //!< Anzahl der Com-Objekte

//!< Höchstes Com-Objekt das bei Info-Senden gesendet wird.
#define OBJ_HIGH_INFO_SEND      21

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
