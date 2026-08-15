/*
 *  tpuart_emulator.h - KNX TP-UART 2 host protocol emulation on top of sblib.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef TPUART_EMULATOR_H_
#define TPUART_EMULATOR_H_

#include <stdint.h>

#include "bcu_tpuart.h"
#include "config.h"

/**
 * Emulates the host interface of a Siemens TP-UART 2.
 *
 * The KNX TP1 side is handled by the Selfbus library, the host side speaks
 * the TP-UART service codes over the LPC1115 UART. See README.md for the
 * supported services and the known deviations from a real transceiver.
 */
class TpUartEmulator
{
public:
    explicit TpUartEmulator(BcuTpUart& bcuTpUart);
    TpUartEmulator() = delete;

    /** Initialize the emulator. Call after the BCU and the serial port are up. */
    void begin();

    /** Processing loop, call from the application's loop(). */
    void loop();

private:
    /** Number of data octets that follow the given host service code. */
    static uint8_t hostServiceDataLength(uint8_t cmd);

    void processHostByte(uint8_t data);
    void handleHostService(uint8_t cmd, const uint8_t* data);

    void handleReset();
    void handleStateRequest();
    void handleDataOctet(uint16_t index, uint8_t data);
    void handleFrameEnd(uint16_t index, uint8_t data);
    void submitFrame(uint16_t length);

    void pollHost();
    void pollKnxReceive();
    void pollKnxTransmit();
    void pollLeds();

    bool queueFree(uint16_t count) const;
    void queueByte(uint8_t data);
    void queueBytes(const uint8_t* data, uint16_t length);
    void drainQueue();

    BcuTpUart& bcu;

    // Host service parser
    uint8_t cmdByte;                //!< service code currently being collected
    uint8_t cmdData[2];             //!< data octets of the current service
    uint8_t cmdDataLen;             //!< number of data octets collected so far
    uint8_t cmdDataExpected;        //!< number of data octets still to collect

    // Frame assembly, host -> KNX
    uint8_t assembleBuffer[TPUART_MAX_FRAME_SIZE]; //!< frame as sent by the host
    uint8_t txOffset;               //!< current U_L_DataOffset, unit 64 octets

    // Frame handed over to sblib. Must stay valid while Bus is sending.
    uint8_t txFrame[TPUART_MAX_FRAME_SIZE];
    bool     txPending;             //!< waiting for the frame to leave the bus
    bool     txSuppressCon;         //!< drop the confirmation, e.g. after a reset
    uint32_t txStartTime;           //!< millis() when the frame was handed over

    // Ring buffer, controller -> host
    uint8_t  txQueue[TPUART_TX_QUEUE_SIZE];
    uint16_t txQueueHead;
    uint16_t txQueueTail;
    uint32_t lastTxByteTime;

    bool    busMonitorMode;         //!< set by U_BusmonReq, cleared by reset only
    bool    busyMode;               //!< set by U_SetBusyReq
    bool    stopMode;               //!< set by U_StopModeReq
    uint8_t errorFlags;             //!< latched flags for the next U_State.ind

    uint32_t knxRxLedOffTime;
    uint32_t hostRxLedOffTime;
};

#endif /* TPUART_EMULATOR_H_ */
