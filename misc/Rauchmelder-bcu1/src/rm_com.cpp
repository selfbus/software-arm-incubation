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

#include <sblib/eib.h>
#include <sblib/serial.h>
#include <sblib/io_pin_names.h>

#include "rm_com.h"
#include "rm_const.h"



// Maximale Anzahl Zeichen einer Nachricht vom Rauchmelder, exklusive STX und ETX
// Dekodiert brauchen je zwei Zeichen ein Byte Platz in recvBuf.
#define RECV_MAX_CHARS 12

// Buffer für eine dekodierte Nachricht vom Rauchmelder
unsigned char recvBuf[RECV_MAX_CHARS >> 1];

// Zähler für die empfangenen Zeichen vom Rauchmelder
int recvCount;


// Hilfsstring für die Umschlüsselung Zahl auf Hex-String
unsigned const char hexDigits[] = "0123456789ABCDEF";

// configure serial pins for Rauchmelder
//Serial serial(PIO3_1, PIO3_0);

/**
 * Serielle Kommunikation mit dem Rauchmelder initialisieren
 */
void rm_serial_init()
{
	serial.setRxPin(PIN_RX);
	serial.setTxPin(PIN_TX);
	serial.begin(9600);
}

/**
 * Prüfen ob Rauchmelder aktiv ist
 *
 * @return 0 wenn Rauchmelder inativ, 1 wenn Rauchmelder aktiv
 */
bool checkRmActivity(void)
{
	pinMode(RM_ACTIVITY_PIN, INPUT | PULL_DOWN);	//Pin als Eingang mit Pulldown Widerstand konfigurieren
	bool ret;
	if(digitalRead(RM_ACTIVITY_PIN)){
		ret = 1;
	}else{
		ret = 0;
	}
	pinMode(RM_ACTIVITY_PIN, INPUT);
	return ret;
}


/**
 * Ein Byte an den Rauchmelder senden.
 */
void rm_send_byte(unsigned char ch)
{
	if(checkRmActivity()){		//prüfen, ob der Rauchmelder auf Bodenplatte gesteckt und somit aktiv ist
		serial.write(ch);
	}
}


/**
 * Eine Nachricht an den Rauchmelder senden.
 */
void rm_send_hexstr(unsigned char* hexstr)
{
	unsigned char checksum = 0;
	unsigned char ch;

	rm_send_byte(STX);

	while (*hexstr)
	{
		ch = *hexstr;
		checksum += ch;
		rm_send_byte(ch);
		++hexstr;
	}

	rm_send_byte(hexDigits[checksum >> 4]);
	rm_send_byte(hexDigits[checksum & 15]);

	rm_send_byte(ETX);
}


/**
 * Einen 1 Byte Befehl an den Rauchmelder senden.
 */
void rm_send_cmd(unsigned char cmd)
{
	unsigned char b, bytes[3];

	b = cmd >> 4;
	bytes[0] = hexDigits[b];

	b = cmd & 0x0f;
	bytes[1] = hexDigits[b];

	bytes[2] = 0;
	rm_send_hexstr(bytes);
}


/**
 * Ein Byte über die Serielle vom Rauchmelder empfangen.
 */
void rm_recv_byte()
{
	char idx;

	unsigned char ch;

	int rec_ch = serial.read();
	if(rec_ch == -1){
		return;
	}else{
		ch = (unsigned char) rec_ch;
	}

	// Am Anfang auf das Start Byte warten
	if (recvCount < 0)
	{
		if (ch == STX)
			++recvCount;
		return;
	}

	idx = recvCount >> 1;

	// Am Ende den Empfang bestätigen und die erhaltene Antwort verarbeiten
	if (ch == ETX)
	{
		rm_send_byte(ACK);

		if (idx > 1)
			rm_process_msg(recvBuf, idx - 1); // Verarbeitung aufrufen (ohne Prüfsumme)

		recvCount = -1;

		return;
	}

	// Bei Überlauf die restlichen Zeichen ignorieren
	if (recvCount >= RECV_MAX_CHARS)
		return;

	// Die empfangenen Zeichen sind ein Hex String.
	// D.h. jeweils zwei Zeichen ergeben ein Byte.
	// In Answer gleich die dekodierten Bytes schreiben.
	//
	// Dieser Algorythmus ist fehlerhaft falls die Anzahl der empfangenen
	// Zeichen ungerade ist.

	if (ch >= '0' && ch <= '9')
		ch -= '0';
	else if (ch >= 'A' && ch <= 'F')
		ch -= 'A' - 10;
	else return; // Ungültige Zeichen ignorieren

	if (recvCount & 1)
	{
		recvBuf[idx] <<= 4;
		recvBuf[idx] |= ch;
	}
	else recvBuf[idx] = ch;

	++recvCount;
}
