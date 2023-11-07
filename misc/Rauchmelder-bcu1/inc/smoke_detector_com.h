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
     * Check if we are currently receiving bytes from the smoke detector
     *
     * @return true if receiving, otherwise false
     */
    bool isReceiving();

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
    bool setAlarmState(RmAlarmState newState);

private:
    SmokeDetectorComCallback *callback;
};

#endif /*SMOKE_DETECTOR_COM_H*/
