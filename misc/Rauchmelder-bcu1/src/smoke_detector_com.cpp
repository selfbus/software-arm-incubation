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

#include "smoke_detector_com.h"
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

/**
 * Serielle Kommunikation mit dem Rauchmelder initialisieren
 */
void rm_serial_init()
{
    recvCount = -1;
    pinMode(RM_COMM_ENABLE_PIN, OUTPUT);
    digitalWrite(RM_COMM_ENABLE_PIN, RM_COMM_ENABLE); // Enable communication on the smoke detector

    serial.setRxPin(PIN_RX);
    serial.setTxPin(PIN_TX);
    pinMode(PIN_RX, SERIAL_RXD | INPUT | PULL_UP); // use internal pull-up resistor to avoid noise then not connected
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

void rm_send_nak()
{
    rm_send_byte(NAK);
}

/**
 * Eine Nachricht an den Rauchmelder senden.
 *
 * Der Befehl wird als Hex String gesendet. Die Prüfsumme wird automatisch
 * berechnet und nach dem Befehl gesendet. Die gesamte Übertragung wird mit
 * STX begonnnen und mit ETX beendet.
 *
 * @param hexstr - die zu sendenden Bytes als Hex String, mit Nullbyte am Ende
 */
void rm_send_hexstr(unsigned char *hexstr)
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

bool rm_send_cmd(RmCommandByte cmd)
{
    if (!serial.enabled())
    {
        return false;
    }

    unsigned char b, bytes[3];

    b = cmd >> 4;
    bytes[0] = hexDigits[b];

    b = cmd & 0x0f;
    bytes[1] = hexDigits[b];

    bytes[2] = 0;
    rm_send_hexstr(bytes);
    return true;
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

void rm_recv_bytes()
{
    if (!serial.enabled())
    {
        return;
    }

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

        ch = (uint8_t) rec_ch;
        switch (ch)
        {
            case STX:
                // It is the magic start byte, (re-)start message reception.
                // It can also be a repetition of a failed previous attempt.
                recvCount = 0;
                continue;

            case NAK: ///\todo Retransmit the last command/bytes
                continue;

            case NUL:
                continue;

            case ETX:
                if (isReceiving()) // ignore random end byte,
                {
                    if ((recvCount & 1) == 0) // recvCount must be a multiple of 2
                    {
                        idx = recvCount >> 1;
                        if (rm_is_valid_message(idx)) // verify checksum incl. minimum length
                        {
                            // Am Ende den Empfang bestätigen und die erhaltene Antwort verarbeiten
                            rm_send_ack();
                            rm_process_msg(recvBuf, idx - 1);
                        }
                    }
                }
                else
                {
                    rm_send_nak();
                }
                rm_cancel_receive();
                continue;

            default:
                break;
        }

        // Ignore random bytes.
        if (recvCount < 0)
            continue;

        idx = recvCount >> 1;

        // Bei Überlauf die restlichen Zeichen ignorieren
        if (recvCount >= RECV_MAX_CHARS)
        {
            rm_cancel_receive();
            continue;
        }

        // The received characters are a hex string, i.e. two characters make one byte.
        // The characters are written as decoded bytes.
        // This algorithm is incorrect if the number of received characters is odd.
        // This check is done finally before calling rm_process_msg.

        if (ch >= '0' && ch <= '9')
            ch -= '0';
        else if (ch >= 'A' && ch <= 'F')
            ch -= 'A' - 10;
        else // ignore invalid characters
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
        case RM_ALARM:
            rm_send_hexstr((unsigned char*) "030210");
            break;

        case RM_TEST_ALARM:
            rm_send_hexstr((unsigned char*) "030280");
            break;

        case RM_NO_ALARM:
            //\todo Does this clear the status byte completely, including local test alarm and battery empty bits?
            rm_send_hexstr((unsigned char*) "030200");
            break;

        default:
            break;
    }
    return (true);
}

bool isReceiving()
{
    return (recvCount >= 0);
}
