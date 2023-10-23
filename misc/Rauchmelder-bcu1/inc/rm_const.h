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

#include <sblib/io_pin_names.h>


// IO Pin assignments
#define RM_COMM_ENABLE_PIN	PIO3_5
#define RM_COMM_ENABLE false // set low to enable smoke detector's serial communication feature

// Eingangspin definieren, an dem die Erkennung der internen Rauchmelder Spannung angeschlossen ist
#define RM_ACTIVITY_PIN PIO0_11
#define RM_IS_ACTIVE true

// Ansteuerung der Spannungsversorgung zur Untertützung des Rauchmelders
#define RM_SUPPORT_VOLTAGE_PIN PIO2_1
#define RM_SUPPORT_VOLTAGE_ON false
#define RM_SUPPORT_VOLTAGE_OFF true // NPN Transistor zieht die Versorgungsspannung herunter

// LED pins
#define LED_BASEPLATE_DETACHED_PIN PIO2_6       //!< LED is on, while base plate is not attached means @ref RM_ACTIVITY_PIN is false / off
#define LED_SUPPLY_VOLTAGE_DISABLED_PIN PIO3_3  //!< LED is on, while supply voltage is "disconnected" means @ref RM_SUPPORT_VOLTAGE_PIN is true

// Sentinel values
#define BATTERY_VOLTAGE_INVALID -1              //!< Value representing an invalid battery voltage

// Time-related values
#define TIMER_INTERVAL_MS 500                   //!< Periodic timer interval in milliseconds (handles all periodic tasks)
#define SUPPLY_VOLTAGE_DELAY_MS 500             //!< Delay time in milliseconds we wait after enabling the 12V supply
#define SUPPLY_VOLTAGE_TIMEOUT_MS 120000        //!< Maximum waiting time in milliseconds to enable 12V supply

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

#define OBJ_NONE                31

// Alarm Vernetzung
#define OBJ_ALARM_BUS			0

// Testalarm Vernetzung
#define OBJ_TALARM_BUS			1

// Alarm / Testalarm rücksetzen
#define OBJ_RESET_ALARM			2

// Status Alarm
#define OBJ_STAT_ALARM			3

// Status verzögerter Alarm
#define OBJ_STAT_ALARM_DELAYED	4

// Status Testalarm
#define OBJ_STAT_TALARM			5

// Rauchmelder Seriennummer
#define OBJ_SERIAL				6

// Rauchmelder Betriebsdauer in Sekunden
#define OBJ_OPERATING_TIME		7

// Rauchkammer Wert
#define OBJ_SMOKEBOX_VALUE		8

// Rauchkammer Verschmutzung
#define OBJ_POLLUTION			9

// Batterie Spannung
#define OBJ_BAT_VOLTAGE			10

// Temperatur
#define OBJ_TEMP				11

// Rauchmelder Fehlercode, siehe ERRCODE_* Konstanten
#define OBJ_ERRCODE				12

// Batterie leer
#define OBJ_BAT_LOW				13

// Rauchmelder defekt
#define OBJ_MALFUNCTION			14

// Anzahl Rauchalarme
#define OBJ_CNT_SMOKEALARM		15

// Anzahl Temperatur Alarme
#define OBJ_CNT_TEMPALARM		16

// Anzahl Testalarme
#define OBJ_CNT_TESTALARM		17

// Anzahl Draht Alarme
#define OBJ_CNT_ALARM_WIRE		18

// Anzahl Bus Alarme
#define OBJ_CNT_ALARM_BUS		19

// Anzahl Draht Testalarme
#define OBJ_CNT_TALARM_WIRE		20

// Anzahl Bus Testalarme
#define OBJ_CNT_TALARM_BUS		21


// Anzahl der Com-Objekte
#define NUM_OBJS                22

// Anzahl der Bytes die benötigt werden um die Com-Objekte als Bits abzubilden
#define NUM_OBJ_FLAG_BYTES ((NUM_OBJS + 7) >> 3)

// Höchstes Com-Objekt das bei Info-Senden gesendet wird.
#define OBJ_HIGH_INFO_SEND      21

// Maximale Anzahl von ComObjekten pro Rauchmelder-Befehl
#define MAX_OBJ_CMD  4

//-----------------------------------------------------------------------------
// Rauchmelder Fehlercodes
//-----------------------------------------------------------------------------
#define ERRCODE_BATLOW		    0x01 //!< Batterie schwach/leer
#define ERRCODE_COMM		    0x02 //!< Rauchmelder antwortet nicht
#define ERRCODE_TEMP1		    0x04 //!< Temperatursensor 1 defekt
#define ERRCODE_TEMP2		    0x08 //!< Temperatursensor 2 defekt
#define ERRCODE_SMOKEBOX	    0x10 //!< Rauchkammer verschmutzt / defekt

//-----------------------------------------------------------------------------
// Datentypen der Rauchmelder Antworten
//-----------------------------------------------------------------------------

// Rauchmelder Antwort: 4 Byte Zahl
#define RM_TYPE_LONG 1

// Rauchmelder Antwort: 2 Byte Zahl
#define RM_TYPE_SHORT 2

// Rauchmelder Antwort: 1 Byte Zahl
#define RM_TYPE_BYTE 3

// Rauchmelder Antwort: 1 Byte Temperatur
#define RM_TYPE_TEMP 4

// Rauchmelder Antwort: 2 Byte Spannung
#define RM_TYPE_MVOLT 5

// Rauchmelder Antwort: 4 Byte 1/4 Sekunden
#define RM_TYPE_QSEC 6

// Rauchmelder Antwort: Kein Typ
#define RM_TYPE_NONE 127


//-----------------------------------------------------------------------------
// Kommunikations Konstanten
//-----------------------------------------------------------------------------

#define THE_UNKNOWN 0x00 //!> The unknown leading byte of the @ref STX start byte
#define STX         0x02 //!> Start byte
#define ETX         0x03 //!> End byte
#define ACK         0x06 //!> Acknowledged byte
#define NAK         0x15 //!< Not acknowledged byte

#endif /*rm_const_h*/
