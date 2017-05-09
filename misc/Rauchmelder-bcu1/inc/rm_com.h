/*
 *  Original written for LPC922:
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  Modified for LPC1115 ARM processor:
 *  Copyright (c) 2017 Oliver Stefan <o.stefan252@googlemail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef rm_com_h
#define rm_com_h

// Eingangspin definieren, an dem die Erkennung der internen Rauchmelder Spannung angeschlossen ist
#define RM_ACTIVITY_PIN PIO1_0

//
// Funktionen für die Kommunikation mit dem Rauchmelder
//

/**
 * Ein Byte über die serielle Schnittstelle vom Rauchmelder empfangen.
 *
 * Wenn die empfangene Nachricht komplett ist dann wird _process_msg()
 * zur Verarbeitung der Nachricht aufgerufen. Diese Funktion muss aus main()
 * aufgerufen werden wenn ein Byte empfangen wurde.
 */
extern void rm_recv_byte();

/**
 * Die empfangene Nachricht vom Rauchmelder verarbeiten.
 *
 * Wird von _receive() aufgerufen wenn eine Nachricht empfangen wurde.
 * Muss vom Verwender implementiert werden.
 *
 * @param bytes - die empfangene Nachricht, ohne STX, ETX, Prüfsumme.
 * @param len - die Anzahl der empfangenen Bytes
 */
extern void rm_process_msg(unsigned char* bytes, unsigned char len);

/**
 * Serielle Kommunikation mit dem Rauchmelder initialisieren
 */
extern void rm_serial_init();

/**
 * Einen 1 Byte Befehl an den Rauchmelder senden.
 *
 * @param cmd - das Befehls-Byte.
 */
extern void rm_send_cmd(unsigned char cmd);

/**
 * Ein Byte an den Rauchmelder senden.
 *
 * @param ch - das zu sendende Byte.
 */
extern void rm_send_byte(unsigned char ch);

/**
 * Eine Nachricht an den Rauchmelder senden.
 *
 * Der Befehl wird als Hex String gesendet. Die Prüfsumme wird automatisch
 * berechnet und nach dem Befehl gesendet. Die gesammte Übertragung wird mit
 * STX begonnnen und mit ETX beendet.
 *
 * @param hexstr - die zu sendenden Bytes als Hex String, mit Nullbyte am Ende
 */
extern void rm_send_hexstr(unsigned char* hexstr);

/**
 * Wenn >= 0 dann wird gerade etwas vom Rauchmelder empfangen
 */
extern int recvCount;


#endif /*rm_com_h*/
