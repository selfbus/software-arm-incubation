/*
 *  Original written for LPC922:
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  Modified for LPC1115 ARM processor:
 *  Copyright (c) 2017-2022 Oliver Stefan <o.stefan252@googlemail.com>
 *
 *  Refactoring and bug fixes:
 *  Copyright (c) 2023 Darthyson <darth@maptrack.de>
 *  Copyright (c) 2023 Thomas Dallmair <dev@thomas-dallmair.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef SMOKE_DETECTOR_COM_H
#define SMOKE_DETECTOR_COM_H

#include <stdint.h>

#include "rm_const.h"

/**
 * Command byte to send to the smoke detector
 */
enum RmCommandByte : uint8_t
{
    serialNumber           = 0x04,
    operatingTime          = 0x09,
    smokeboxData           = 0x0B,
    batteryTemperatureData = 0x0C,
    numberAlarms_1         = 0x0D,
    numberAlarms_2         = 0x0E,
    status                 = 0x02
};

class SmokeDetectorComCallback
{
public:
    /**
     * Received a message from the smoke detector.
     *
     * Called by @ref SmokeDetectorCom::receiveBytes() when a message is received from the device.
     *
     * @param bytes - received message without STX, ETX, checksum
     * @param len - number of received bytes
     */
    virtual void receivedMessage(uint8_t *bytes, int8_t len) = 0;
};

/**
 * Communication with the smoke detector
 */
class SmokeDetectorCom
{
public:
    SmokeDetectorCom(SmokeDetectorComCallback *callback);

    /**
     * Initialize serial communication with smoke detector
     */
    void initSerialCom();

    /**
     * Receive all bytes from the smoke detector via serial port.
     * This function must be called continuously from the main loop to receive transmitted bytes.
     * When the received message is complete, this calls @ref SmokeDetectorComCallback::receivedMessage()
     * to process the message.
     */
    void receiveBytes();

    /**
     * Send command @ref cmd to the smoke detector.\n
     * Receiving and processing the response from the smoke detector is done in @ref SmokeDetectorComCallback::receivedMessage().
     *
     * @param cmd - @ref RmCommandByte to send.
     * @return True if command was sent, otherwise false.
     */
    bool sendCommand(RmCommandByte cmd);

    /**
     * Request the smoke detector to enter a specific remote alarm state.
     */
    void setAlarmState(RmAlarmState newState);

private:
    /**
     * Check if we are currently receiving bytes from the smoke detector
     *
     * @return true if receiving, otherwise false
     */
    bool isReceiving();

    /**
     * Cancel an ongoing message reception, e.g. due to timeout.
     */
    void cancelReceive();

    /**
     * Validate the checksum of the message in @ref recvBuf.
     */
    bool isValidMessage(uint8_t length);

    /**
     * Send a byte to the smoke detector.
     *
     * @param b - the byte to send.
     */
    void sendByte(uint8_t b);

    /**
     * Send an ACK to the smoke detector.
     */
    void sendAck();

    /**
     * Send a NAK to the smoke detector.
     */
    void sendNak();

    /**
     * Send a message to the smoke detector.
     *
     * The command is transmitted as a hex string. The checksum is calculated and
     * appended. The whole transmission is initiated with STX and finalized with ETX.
     *
     * @param hexstr - the bytes to send as hex string, with terminating NUL byte
     */
    void sendHexstring(const uint8_t *hexstr);

private:
    // Maximum number of characters of a message from the smoke detector, excluding STX and ETX.
    // Two characters take equate to one byte in @ref recvBuf.
    static constexpr int RecvMaxCharacters = 12;

    // Timeout of serial port communication.
    static constexpr int RecvTimeoutMs = 3000;

    // Lookup table for translation number to hex string
    const uint8_t HexDigits[17];

private:
    SmokeDetectorComCallback *callback;

    // Buffer for storing the decoded message from smoke detector
    uint8_t recvBuf[RecvMaxCharacters >> 1];

    // Number of received characters from smoke detector
    int recvCount;

    // Last time a byte was received from the serial port.
    uint32_t lastSerialRecvTime;
};

#endif /*SMOKE_DETECTOR_COM_H*/
