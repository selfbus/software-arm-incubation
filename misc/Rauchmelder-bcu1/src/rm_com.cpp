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

#include <stdint.h>
#include <sblib/serial.h>
#include <sblib/digital_pin.h>
#include <sblib/io_pin_names.h>
#include <sblib/timer.h>

#include "rm_com.h"
#include "rm_const.h"



// Maximale Anzahl Zeichen einer Nachricht vom Rauchmelder, exklusive STX und ETX
// Dekodiert brauchen je zwei Zeichen ein Byte Platz in recvBuf.
#define RECV_MAX_CHARS 12

// Buffer für eine dekodierte Nachricht vom Rauchmelder
uint8_t recvBuf[RECV_MAX_CHARS >> 1];

// Zähler für die empfangenen Zeichen vom Rauchmelder
int recvCount;

// Last time a byte was received from the serial port.
uint32_t lastSerialRecvTime = 0;

// Timeout of serial port communication.
#define RECV_TIMEOUT_MS (3000)


// Hilfsstring für die Umschlüsselung Zahl auf Hex-String
unsigned const char hexDigits[] = "0123456789ABCDEF";

// configure serial pins for Rauchmelder
//Serial serial(PIO3_1, PIO3_0);

/**
 * Serielle Kommunikation mit dem Rauchmelder initialisieren
 */
void rm_serial_init()
{
    recvCount = -1;
    serial.setRxPin(PIN_RX);
	serial.setTxPin(PIN_TX);
	serial.begin(9600);
}

/**
 * Ein Byte an den Rauchmelder senden.
 *
 * @param ch - das zu sendende Byte.
 */
void rm_send_byte(unsigned char ch)
{
	serial.write(ch);
}

void rm_send_ack()
{
    rm_send_byte(ACK);
}

/**
 * Eine Nachricht an den Rauchmelder senden.
 *
 * Der Befehl wird als Hex String gesendet. Die Prüfsumme wird automatisch
 * berechnet und nach dem Befehl gesendet. Die gesammte Übertragung wird mit
 * STX begonnnen und mit ETX beendet.
 *
 * @param hexstr - die zu sendenden Bytes als Hex String, mit Nullbyte am Ende
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
 * Cancel an ongoing message reception, e.g. due to timeout.
 */
void rm_cancel_receive()
{
    recvCount = -1;
}

/**
 * Validate the checksum of the message in recvBuf.
 */
bool rm_is_valid_message(uint8_t length)
{
    // Message has at least one control code and a checksum byte.
    if (length < 2)
        return false;

    // There are a few invalid messages captured in RM_Protokoll.txt:
    //
    //      <STX>822080F4<ETX>
    //      C2 30 00 00 00 FD
    //      <STX>CC01DB52533B<ETX>
    //      <STX>CE020448<ETX>
    //
    // Also, every entry in the long list of the "02 - Status abfragen" section was
    // actually recorded with first byte 82 instead of C2, and consequently the
    // checksums in this list are all wrong.
    // Nevertheless, the description is correct and all other captured messages
    // have correct checksums, so it's pretty safe to throw away messages with an
    // incorrect checksum.

    uint8_t expectedChecksum = 0;

    for (auto i = 0; i < length - 1; ++i)
    {
        uint8_t b = recvBuf[i];
        expectedChecksum += hexDigits[b >> 4] + hexDigits[b & 0x0F];
    }

    return expectedChecksum == recvBuf[length - 1];
}

/**
 * Ein Byte über die Serielle vom Rauchmelder empfangen.
 */
void rm_recv_byte()
{
    if (isReceiving() && elapsed(lastSerialRecvTime) > RECV_TIMEOUT_MS)
    {
        rm_cancel_receive();
    }

    uint32_t count = serial.available();
    if (count == 0)
    {
        return;
    }

    uint8_t idx;
    uint8_t ch;

    int rec_ch;
    while ((rec_ch = serial.read()) > -1)
    {
        lastSerialRecvTime = millis();
        ch = (unsigned char) rec_ch;

        // If it is the magic start byte, (re-)start message reception.
        // It can also be a repetition of a failed previous attempt.
        if (ch == STX)
        {
            recvCount = 0;
            continue;
        }

        // Ignore random bytes.
        if (recvCount < 0)
            continue;

        idx = recvCount >> 1;

        // Am Ende den Empfang bestätigen und die erhaltene Antwort verarbeiten
        if (ch == ETX)
        {
            rm_send_ack();

            if (rm_is_valid_message(idx))
                rm_process_msg(recvBuf, idx - 1); // Verarbeitung aufrufen

            rm_cancel_receive();
            return;
        }

        // Bei Überlauf die restlichen Zeichen ignorieren
        if (recvCount >= RECV_MAX_CHARS)
        {
            rm_cancel_receive();
            continue;
        }

        // Die empfangenen Zeichen sind ein Hex String.
        // D.h. jeweils zwei Zeichen ergeben ein Byte.
        // In Answer gleich die dekodierten Bytes schreiben.
        //
        // Dieser Algorithmus ist fehlerhaft falls die Anzahl der empfangenen
        // Zeichen ungerade ist.

        if (ch >= '0' && ch <= '9')
            ch -= '0';
        else if (ch >= 'A' && ch <= 'F')
            ch -= 'A' - 10;
        else // Ungültige Zeichen ignorieren
        {
            rm_cancel_receive();
            continue;
        }

        if (recvCount & 1)
        {
            recvBuf[idx] <<= 4;
            recvBuf[idx] |= ch;
        }
        else
        {
            recvBuf[idx] = ch;
        }

        ++recvCount;
    } // while
}

bool rm_set_alarm_state(RmAlarmState newState)
{
    switch (newState)
    {
        case RM_ALARM :
            rm_send_hexstr((unsigned char*)"030210");
            break;

        case RM_TEST_ALARM :
            rm_send_hexstr((unsigned char*)"030280");
            break;

        case RM_NO_ALARM :
            rm_send_hexstr((unsigned char*)"030200");
            break;

        default : ;
    }
    return (true);
}

bool isReceiving()
{
    return (recvCount >= 0);
}
