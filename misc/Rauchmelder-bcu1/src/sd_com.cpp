/*
 *  Original written for LPC922:
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  Modified for LPC1115 ARM processor:
 *  Copyright (c) 2017 Oliver Stefan <o.stefan252@googlemail.com>
 *
 *  Refactoring and bug fixes:
 *  Copyright (c) 2023 Darthyson <darth@maptrack.de>
 *  Copyright (c) 2023 Thomas Dallmair <dev@thomas-dallmair.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <stdint.h>
#include <sblib/serial.h>
#include <sblib/digital_pin.h>
#include <sblib/io_pin_names.h>

#include "sd_com.h"
#include "sd_shared_enums.h"


SmokeDetectorCom::SmokeDetectorCom(SmokeDetectorComCallback *callback)
    : HexDigits("0123456789ABCDEF"),
      callback(callback)
{
    recvCount = -1;
    receiveTimeout.stop();
    clearSendBuffer();
    confirmationTimeout.stop();
    lastSentCommand = {};
    ackTimeout.stop();
    capacitorChargeTimeout.stop();
}

/**
 * Initialize serial communication with smoke detector
 */
void SmokeDetectorCom::initSerialCom()
{
    // use internal pull-up resistor to avoid noise when not connected
    serial.setRxPin(PIN_RX);
    serial.setTxPin(PIN_TX);
    pinMode(PIN_RX, SERIAL_RXD | INPUT | PULL_UP);
    serial.begin(9600);
}

void SmokeDetectorCom::end()
{
    serial.end();
    pinMode(PIN_TX, INPUT | OPEN_DRAIN);
    pinMode(PIN_RX, INPUT | OPEN_DRAIN);
}

/**
 * Check whether one of the timeouts expired and act accordingly.
 */
void SmokeDetectorCom::loopCheckTimeouts()
{
    if (confirmationTimeout.expired())
    {
        repeatMessageOrReportTimeout();
    }

    if (receiveTimeout.expired())
    {
        receivedMessageWithFailure();
        encounteredTimeout();
    }

    if (ackTimeout.expired())
    {
        finalizeSuccessfulMessageReception();
    }
}

/**
 * Receive all bytes from the smoke detector via serial port.
 * This function must be called continuously from the main loop to receive transmitted bytes.
 * When the received message is complete, this calls @ref SmokeDetectorComCallback::receivedMessage()
 * to process the message.
 */
void SmokeDetectorCom::receiveBytes()
{
    if (!serial.enabled())
    {
        return;
    }

    auto count = serial.available();
    if (count == 0)
    {
        return;
    }

    int rec_ch;
    while ((rec_ch = serial.read()) > -1)
    {
        auto idx = static_cast<uint8_t>(recvCount >> 1);
        auto ch = static_cast<uint8_t>(rec_ch);
        switch (ch)
        {
            case ACK:
                confirmationTimeout.stop();
                clearSendBuffer();
                continue;

            case NAK:
                confirmationTimeout.stop();
                repeatMessageOrReportTimeout();
                continue;

            case NUL:
                continue;

            case STX:
                // It is the magic start byte, (re-)start message reception.
                // It can also be a repetition of a failed previous attempt.
                recvCount = 0;
                receiveTimeout.start(ReceiveTimeoutMs);
                continue;

            case ETX:
                // Encountered an end byte. If we saw STX beforehand, and recvCount is a multiple of 2,
                // and the length and checksum are valid, it is a valid message.
                if (isReceiving() && (recvCount & 1) == 0 && isValidMessage(idx))
                {
                    // Acknowledge reception and process message.
                    receivedMessageSuccessfully(idx - 1);
                }
                else
                {
                    // Guard against transmission errors of received STX bytes.
                    receivedMessageWithFailure();
                }
                receiveTimeout.stop();
                continue;

            default:
                break;
        }

        // Ignore random bytes.
        if (recvCount < 0)
            continue;

        // On overflow, ignore excess characters
        if (recvCount >= RecvMaxCharacters)
        {
            receivedMessageWithFailure();
            continue;
        }

        // The received characters are a hex string, i.e. two characters make one byte.
        // The characters are written as decoded bytes.
        // This algorithm is incorrect if the number of received characters is odd.
        // This check is done finally before calling callback->receivedMessage().

        if (ch >= '0' && ch <= '9')
            ch -= '0';
        else if (ch >= 'A' && ch <= 'F')
            ch -= 'A' - 10;
        else // ignore invalid characters
        {
            receivedMessageWithFailure();
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

/**
 * Send command @ref cmd to the smoke detector.\n
 * Receiving and processing the response from the smoke detector is done in @ref SmokeDetectorComCallback::receivedMessage().
 *
 * @param cmd - @ref RmCommandByte to send.
 * @return True if command was sent, otherwise false.
 */
bool SmokeDetectorCom::sendCommand(RmCommandByte cmd)
{
    if (!serial.enabled() || isReceiving() || isSending() || !canSend())
    {
        return false;
    }

    auto b = static_cast<std::underlying_type_t<RmCommandByte>>(cmd);

    sendBuf[0] = HexDigits[b >> 4];
    sendBuf[1] = HexDigits[b & 0x0f];
    sendBuf[2] = 0;

    lastSentCommand = cmd;
    sendMessage();
    return true;
}

void SmokeDetectorCom::setAlarmState(RmAlarmState newState)
{
    // While waiting for an answer we don't process alarms to avoid overlapping message exchanges.
    // As a message exchange is fast and this is called from the main loop that's fine.
    if (isReceiving() || isSending() || !canSend())
        return;

    switch (newState)
    {
        case RmAlarmState::alarm:
            memcpy(sendBuf, "030210", 7);
            break;

        case RmAlarmState::testAlarm:
            memcpy(sendBuf, "030280", 7);
            break;

        case RmAlarmState::noAlarm:
            memcpy(sendBuf, "030200", 7);
            break;

        default:
            return;
    }

    lastSentCommand = {};
    sendMessage();
}

/**
 * Check if we are currently receiving bytes from the smoke detector
 *
 * @return true if receiving, otherwise false
 */
bool SmokeDetectorCom::isReceiving()
{
    return (recvCount >= 0);
}

/**
 * Finalize an ongoing message reception, e.g. because it succeeded or due to timeout.
 */
void SmokeDetectorCom::finalizeReceive()
{
    recvCount = -1;
    capacitorChargeTimeout.start(CapacitorChargeTimeoutMs);
}

/**
 * Validate the checksum of the message in @ref recvBuf.
 */
bool SmokeDetectorCom::isValidMessage(uint8_t length)
{
    // Message has at least one control code and a checksum byte.
    if (length < 2)
        return false;

    // There are a few invalid messages captured in rm_protokoll.txt:
    //
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
        auto b = recvBuf[i];
        expectedChecksum += HexDigits[b >> 4];
        expectedChecksum += HexDigits[b & 0x0F];
    }

    return expectedChecksum == recvBuf[length - 1];
}

/**
 * Check if we are currently waiting for the battery support capacitor to
 * charge or are ready to send the next message.
 *
 * @return true if ready to send, otherwise false
 */
bool SmokeDetectorCom::canSend()
{
    return capacitorChargeTimeout.stopped() || capacitorChargeTimeout.expired();
}

/**
 * Check if we are currently sending a message to the smoke detector,
 * i.e. there is valid content in @ref sendBuf. Gets cleared after successful
 * transmission as well as after the first repetition.
 *
 * @return true if sending, otherwise false
 */
bool SmokeDetectorCom::isSending()
{
    return (sendBuf[0] != 0);
}

/**
 * Clears @ref sendBuf.
 */
void SmokeDetectorCom::clearSendBuffer()
{
    sendBuf[0] = 0;
}

/**
 * Sends the message in @ref sendBuf again (first time) or reports a message
 * timeout (from second try on).
 */
void SmokeDetectorCom::repeatMessageOrReportTimeout()
{
    if (isSending())
    {
        // Retransmit the last message stored in sendBuf, but only retry once.
        sendMessage();
        clearSendBuffer();
    }
    else
    {
        encounteredTimeout();
    }
}

/**
 * Notifies @ref callback that a timeout occurred.
 */
void SmokeDetectorCom::encounteredTimeout()
{
    callback->timedOut(lastSentCommand);
    lastSentCommand = {};
}

/**
 * Send a byte to the smoke detector.
 *
 * @param b - the byte to send.
 */
void SmokeDetectorCom::sendByte(uint8_t b)
{
    serial.write(b);
}

/**
 * Forward message to @ref callback for processing and start the timeout for sending ACK.
 */
void SmokeDetectorCom::receivedMessageSuccessfully(uint8_t length)
{
    if (lastSentCommand.has_value() && lastSentCommand.value() == static_cast<RmCommandByte>(recvBuf[0] & 0x0f))
    {
        lastSentCommand = {};
    }
    callback->receivedMessage(recvBuf, length);
    ackTimeout.start(AckTimeoutMs);
}

/**
 * Send ACK to the smoke detector and prepare for next message.
 */
void SmokeDetectorCom::finalizeSuccessfulMessageReception()
{
    sendByte(ACK);
    finalizeReceive();
}

/**
 * Send a NAK to the smoke detector.
 */
void SmokeDetectorCom::receivedMessageWithFailure()
{
    sendByte(NAK);
    finalizeReceive();
}

/**
 * Send a message to the smoke detector.
 *
 * The command is transmitted as a hex string in @ref sendBuf. The checksum is calculated and
 * appended. The whole transmission is initiated with STX and finalized with ETX.
 */
void SmokeDetectorCom::sendMessage()
{
    uint8_t checksum = 0;
    uint8_t b;
    uint8_t *bufPtr = sendBuf;

    sendByte(STX);

    while (*bufPtr)
    {
        b = *bufPtr;
        checksum += b;
        sendByte(b);
        ++bufPtr;
    }

    sendByte(HexDigits[checksum >> 4]);
    sendByte(HexDigits[checksum & 15]);

    sendByte(ETX);

    confirmationTimeout.start(ConfirmationTimeoutMs);
}
