/*
 *  Original written for LPC922:
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  Modified for LPC1115 ARM processor:
 *  Copyright (c) 2017-2022 Oliver Stefan <o.stefan252@googlemail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef rm_com_h
#define rm_com_h

#include <stdint.h>

// IO Pin assignments
#define RM_COMM_ENABLE_PIN	PIO3_5
#define RM_COMM_ENABLE false // set low to enable smoke detector's serial communication feature
#define RM_COMM_DISABLE true

// Eingangspin definieren, an dem die Erkennung der internen Rauchmelder Spannung angeschlossen ist
#define RM_ACTIVITY_PIN PIO0_11
#define RM_IS_ACTIVE true
#define RM_IS_NOT_ACTIVE false

// Ansteuerung der Spannungsversorgung zur Untertützung des Rauchmelders
#define RM_SUPPORT_VOLTAGE_PIN PIO2_1
#define RM_SUPPORT_VOLTAGE_ON false
#define RM_SUPPORT_VOLTAGE_OFF true // NPN Transistor zieht die Versorgungsspannung herunter

/**
 * Smoke detector alarm states
 */
enum RmAlarmState : uint8_t
{
    RM_NO_ALARM,    //!< normal state, no alarm and no test alarm
    RM_ALARM,       //!< alarm state
    RM_TEST_ALARM   //!< test alarm state
};

//
// Funktionen für die Kommunikation mit dem Rauchmelder
//

bool rm_set_alarm_state(RmAlarmState newState);

/**
 * Ein Byte über die serielle Schnittstelle vom Rauchmelder empfangen.
 *
 * Wenn die empfangene Nachricht komplett ist dann wird _process_msg()
 * zur Verarbeitung der Nachricht aufgerufen. Diese Funktion muss aus main()
 * aufgerufen werden wenn ein Byte empfangen wurde.
 */
bool rm_recv_byte();

/**
 * Die empfangene Nachricht vom Rauchmelder verarbeiten.
 *
 * Wird von _receive() aufgerufen wenn eine Nachricht empfangen wurde.
 * Muss vom Verwender implementiert werden.
 *
 * @param bytes - die empfangene Nachricht, ohne STX, ETX, Prüfsumme.
 * @param len - die Anzahl der empfangenen Bytes
 */
void rm_process_msg(unsigned char* bytes, unsigned char len);

/**
 * Serielle Kommunikation mit dem Rauchmelder initialisieren
 */
void rm_serial_init();

/**
 * Einen 1 Byte Befehl an den Rauchmelder senden.
 *
 * @param cmd - das Befehls-Byte.
 */
void rm_send_cmd(unsigned char cmd);

/**
 * Ein ACK an den Rauchmelder senden.
 *
 */
void rm_send_ack();

/**
 * Check if we are currently receiving bytes from the smoke detector
 *
 * @return true if receiving, otherwise false
 */
bool isReceiving();


#endif /*rm_com_h*/
