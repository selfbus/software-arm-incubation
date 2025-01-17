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
#include <optional>
#include <sblib/timeout.h>

#include "sd_shared_enums.h"

/**
 * Command byte to send to the smoke detector
 */
enum class RmCommandByte : uint8_t
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
    virtual void receivedMessage(uint8_t *bytes, uint8_t len) = 0;

    /**
     * Communication with smoke detector timed out.
     *
     * Called by @ref SmokeDetectorCom::receiveBytes() or SmokeDetectorCom::loopCheckTimeouts()
     * when communication with the device times out.
     *
     * @param command The sent command that timed out, or a missing value if the sent command
     *                was triggered by @ref SmokeDetectorCom::setAlarmState or the device itself
     */
    virtual void timedOut(std::optional<RmCommandByte> command) = 0;
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

    void end();

    /**
     * Check whether one of the timeouts expired and act accordingly.
     */
    void loopCheckTimeouts();

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
     * Finalize an ongoing message reception, e.g. because it succeeded or due to timeout.
     */
    void finalizeReceive();

    /**
     * Validate the checksum of the message in @ref recvBuf.
     */
    bool isValidMessage(uint8_t length);

    /**
     * Check if we are currently waiting for the battery support capacitor to
     * charge or are ready to send the next message.
     *
     * @return true if ready to send, otherwise false
     */
    bool canSend();

    /**
     * Check if we are currently sending a message to the smoke detector,
     * i.e. there is valid content in @ref sendBuf. Gets cleared after successful
     * transmission as well as after the first repetition.
     *
     * @return true if sending, otherwise false
     */
    bool isSending();

    /**
     * Clears @ref sendBuf.
     */
    void clearSendBuffer();

    /**
     * Sends the message in @ref sendBuf again (first time) or reports a message
     * timeout (from second try on).
     */
    void repeatMessageOrReportTimeout();

    /**
     * Notifies @ref callback that a timeout occurred.
     */
    void encounteredTimeout();

    /**
     * Send a byte to the smoke detector.
     *
     * @param b - the byte to send.
     */
    void sendByte(uint8_t b);

    /**
     * Forward message to @ref callback for processing and start the timeout for sending ACK.
     */
    void receivedMessageSuccessfully(uint8_t length);

    /**
     * Send ACK to the smoke detector and prepare for next message.
     */
    void finalizeSuccessfulMessageReception();

    /**
     * Send a NAK to the smoke detector.
     */
    void receivedMessageWithFailure();

    /**
     * Send a message to the smoke detector.
     *
     * The command is transmitted as a hex string in @ref sendBuf. The checksum is calculated and
     * appended. The whole transmission is initiated with STX and finalized with ETX.
     */
    void sendMessage();

private:
    // Maximum number of characters of a message from the smoke detector, excluding STX and ETX.
    // Two characters take equate to one byte in @ref recvBuf.
    static constexpr int RecvMaxCharacters = 12;

    // Time we give the smoke detector for transmitting a full message, from STX to ETX.
    static constexpr int ReceiveTimeoutMs = 250;

    // Maximum number of characters of a message to send to the smoke detector, excluding STX and ETX and checksum.
    static constexpr int SendMaxCharacters = 6;

    // Time we give the smoke detector to respond with ACK/NAK before we repeat or report a timeout.
    static constexpr int ConfirmationTimeoutMs = 500;

    // Time we wait before we send an ACK to the device
    static constexpr int AckTimeoutMs = 250;

    // Time we reserve for the capacitor to charge before we send the next request to the device
    static constexpr int CapacitorChargeTimeoutMs = 1500;

    // Lookup table for translation number to hex string
    const uint8_t HexDigits[17];

    enum RmControlByte
    {
        NUL = 0x00, //!> Null byte (prefix of @ref STX start byte)
        STX = 0x02, //!> Start byte
        ETX = 0x03, //!> End byte
        ACK = 0x06, //!> Acknowledged byte
        NAK = 0x15  //!< Not acknowledged byte
    };

private:
    SmokeDetectorComCallback *callback;

    // Buffer for storing the decoded message from smoke detector
    uint8_t recvBuf[RecvMaxCharacters >> 1];

    // Number of received characters from smoke detector
    int recvCount;

    // Timeout for receiving a full message before we reject an incomplete one with NAK
    // Guards against transmission errors of received ETX bytes
    Timeout receiveTimeout;

    // Buffer for storing the message to be sent to the smoke detector
    uint8_t sendBuf[SendMaxCharacters + 1];

    // Timeout for receiving ACK/NAK before we repeat the message or report a timeout
    // Guards against transmission errors of sent STX/ETX or received ACK/NAK bytes
    Timeout confirmationTimeout;

    // Last command sent to the smoke detector
    std::optional<RmCommandByte> lastSentCommand;

    // After receiving a message from the smoke detector, wait for some time before
    // sending ACK
    Timeout ackTimeout;

    // After receiving a message from the smoke detector, allot some time
    // for the battery support capacitor to charge
    Timeout capacitorChargeTimeout;
};

#endif /*SMOKE_DETECTOR_COM_H*/
