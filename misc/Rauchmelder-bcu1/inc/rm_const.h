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


const uint32_t TIMER_INTERVAL_MS = 500; //!< Periodic timer interval in milliseconds (handles all periodic tasks)
const uint32_t STARTUP_DELAY_MS = 500;  //!< Delay time in milliseconds we wait for the serial port initialization of the smoke detector

// counters are in half seconds (1=0.5s, 120=60.0s)
#ifdef DEBUG
    const uint8_t DEFAULT_EVENTTIME = 60;
    const uint8_t DEFAULT_SERIAL_COMMAND_TIME = 7;
    const uint8_t DEFAULT_KNX_OBJECT_TIME = 1;
#else
    const uint8_t DEFAULT_EVENTTIME = 120;  // Initialisierung auf 1 Minute (sonst wird im Timer Interrupt 0 minus 1 durchgeführt)
    const uint8_t DEFAULT_SERIAL_COMMAND_TIME = 15; // half seconds
    const uint8_t DEFAULT_KNX_OBJECT_TIME = 3; // half seconds
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
#define ERRCODE_NOT_ATTACHED    0x20 //!< Smoke detector not attached to base plate

//-----------------------------------------------------------------------------
// Befehle an den Rauchmelder
//-----------------------------------------------------------------------------

// Gira Command: Seriennummer abfragen
#define RM_CMD_SERIAL  0

// Gira Command: Betriebszeit abfragen
#define RM_CMD_OPERATING_TIME  1

// Gira Command: Rauchkammer Daten abfragen
#define RM_CMD_SMOKEBOX 2

// Gira Command: Batteriespannung und Temperaturen
#define RM_CMD_BATTEMP  3

// Gira Command: Anzahl der Alarme #1 abfragen
#define RM_CMD_NUM_ALARMS  4

// Gira Command: Anzahl der Alarme #2 abfragen
#define RM_CMD_NUM_ALARMS_2  5

// Anzahl der Gira Commands
#define RM_CMD_COUNT  6

// Gira Command: intern behandelt, kein Befehl an den Rauchmelder
#define RM_CMD_INTERNAL 14

// Gira Command: kein Befehl
#define RM_CMD_NONE  15


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

// Start Byte
#define STX		0x02

// Ende Byte
#define ETX		0x03

// Acknowledged
#define ACK	 	0x06

// Not Acknowledged
#define NAK	 	0x15


const int8_t BATTERY_VOLTAGE_INVALID = -1; //!< Value representing a invalid battery voltage

#endif /*rm_const_h*/
