/*
 *  tpuart_emulator.cpp - KNX TP-UART 2 host protocol emulation on top of sblib.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/bits.h>
#include <sblib/digital_pin.h>
#include <sblib/serial.h>
#include <sblib/timer.h>

#include <cstring>

#include "tpuart_defs.h"
#include "tpuart_emulator.h"

#define TX_QUEUE_MASK (TPUART_TX_QUEUE_SIZE - 1)

TpUartEmulator::TpUartEmulator(BcuTpUart& bcuTpUart) :
    bcu(bcuTpUart),
    cmdByte(0),
    cmdDataLen(0),
    cmdDataExpected(0),
    txOffset(0),
    txPending(false),
    txSuppressCon(false),
    txStartTime(0),
    txQueueHead(0),
    txQueueTail(0),
    lastTxByteTime(0),
    busMonitorMode(false),
    busyMode(false),
    stopMode(false),
    errorFlags(0),
    knxRxLedOffTime(0),
    hostRxLedOffTime(0)
{
    cmdData[0] = 0;
    cmdData[1] = 0;
    memset(assembleBuffer, 0, sizeof(assembleBuffer));
    memset(txFrame, 0, sizeof(txFrame));
}

void TpUartEmulator::begin()
{
    lastTxByteTime = millis();
    bcu.setLinkLayerActive(true);
}

void TpUartEmulator::loop()
{
    pollHost();
    pollKnxTransmit();
    pollKnxReceive();
    drainQueue();
    pollLeds();
}

/*
 * ---------------------------------------------------------------------------
 * Host -> controller
 * ---------------------------------------------------------------------------
 */

uint8_t TpUartEmulator::hostServiceDataLength(uint8_t cmd)
{
    if (cmd >= U_L_DATA_START_CONT_REQ) // 0x80..0xFF
    {
        if (cmd <= 0xBF)
        {
            return (1); // U_L_DataStart/Cont, followed by the frame octet
        }
        if (cmd == U_SET_ADDRESS_REQ_ALT)
        {
            return (2);
        }
        if (cmd == U_SET_REPETITION_REQ)
        {
            return (1);
        }
        return (0);
    }

    if (cmd >= U_L_DATA_END_REQ) // 0x40..0x7F
    {
        return (1); // U_L_DataEnd, followed by the last frame octet
    }

    if (cmd == U_SET_ADDRESS_REQ) // 0x28
    {
        return (2);
    }

    if ((cmd >= U_INT_REG_WR_REQ) && (cmd <= 0x2B)) // NCN512x register write
    {
        return (1);
    }

    // Repetition counter. The OpenKNX stack sends this whenever the configured
    // NACK/BUSY repetition count differs from the default, but only when it is
    // NOT built for the NCN512x (there the same setting uses 0xF2).
    if (cmd == U_MXRSTCNT_REQ) // 0x24
    {
        return (1);
    }

    return (0);
}

void TpUartEmulator::processHostByte(uint8_t data)
{
    if (cmdDataExpected != 0)
    {
        cmdData[cmdDataLen++] = data;
        if (cmdDataLen < cmdDataExpected)
        {
            return;
        }
        cmdDataExpected = 0;
        handleHostService(cmdByte, cmdData);
        return;
    }

    cmdByte = data;
    cmdDataLen = 0;
    cmdDataExpected = hostServiceDataLength(data);
    if (cmdDataExpected == 0)
    {
        handleHostService(cmdByte, nullptr);
    }
}

void TpUartEmulator::handleHostService(uint8_t cmd, const uint8_t* data)
{
    // Frame transmission services are the most frequent ones, handle them first.
    if ((cmd >= U_L_DATA_START_CONT_REQ) && (cmd <= 0xBF))
    {
        handleDataOctet((uint16_t)txOffset * 64u + (cmd & 0x3F), data[0]);
        return;
    }

    if ((cmd >= U_L_DATA_END_REQ) && (cmd <= 0x7F))
    {
        handleFrameEnd((uint16_t)txOffset * 64u + (cmd & 0x3F), data[0]);
        return;
    }

    if ((cmd >= U_L_DATA_OFFSET_REQ) && (cmd <= 0x0C))
    {
        txOffset = cmd & 0x07;
        return;
    }

    if ((cmd >= U_ACK_INFORMATION_REQ) && (cmd <= 0x17))
    {
        // The acknowledge decision cannot be delegated to the host: the KNX
        // acknowledge slot opens 15 bit times after the last frame octet, while
        // sblib only reports a telegram once it is completely received. The
        // library therefore acknowledges autonomously, see README.md.
        return;
    }

    if ((cmd >= U_SET_ADDRESS_REQ) && (cmd <= 0x2B))
    {
        if (cmd == U_SET_ADDRESS_REQ)
        {
            bcu.setOwnAddress(makeWord(data[0], data[1]));
        }
        return;
    }

    switch (cmd)
    {
    case U_RESET_REQ:
        handleReset();
        break;

    case U_STATE_REQ:
        handleStateRequest();
        break;

    case U_SET_BUSY_REQ:
        busyMode = true;
        bcu.setLinkLayerActive(false);
        break;

    case U_QUIT_BUSY_REQ:
        busyMode = false;
        if (!busMonitorMode)
        {
            bcu.setLinkLayerActive(true);
        }
        break;

    case U_BUSMON_REQ:
        // In bus monitor mode nothing may be acknowledged on the bus.
        busMonitorMode = true;
        bcu.setLinkLayerActive(false);
        break;

    case U_SYSTEM_STATE_REQ:
        queueByte(U_SYSTEM_STAT_IND);
        queueByte(stopMode ? 0x01 : 0x00);
        break;

    case U_STOP_MODE_REQ:
        if (!stopMode)
        {
            bcu.bus->pause(true);
            stopMode = true;
        }
        queueByte(U_STOP_MODE_IND);
        break;

    case U_EXIT_STOP_MODE_REQ:
        if (stopMode)
        {
            bcu.bus->resume();
            stopMode = false;
        }
        break;

    case U_SET_ADDRESS_REQ_ALT:
        bcu.setOwnAddress(makeWord(data[0], data[1]));
        break;

    default:
        // U_ProductId, U_Configure, U_IntRegRd, U_SetRepetition and anything
        // unknown are silently accepted. Their data octets, if any, have
        // already been consumed by hostServiceDataLength().
        break;
    }
}

void TpUartEmulator::handleReset()
{
    if (stopMode)
    {
        bcu.bus->resume();
        stopMode = false;
    }

    cmdDataExpected = 0;
    cmdDataLen = 0;
    txOffset = 0;
    busMonitorMode = false;
    busyMode = false;
    errorFlags = 0;
    bcu.setLinkLayerActive(true);

    // Drop everything that is still queued towards the host.
    txQueueHead = 0;
    txQueueTail = 0;
    bcu.bus->discardReceivedTelegram();

    if (txPending)
    {
        // A frame may still be on its way out. Keep the buffer reserved, but
        // do not report its confirmation to the host after the reset.
        txSuppressCon = true;
    }

    queueByte(U_RESET_IND);
}

void TpUartEmulator::handleStateRequest()
{
    queueByte((uint8_t)(U_STATE_IND | errorFlags));
    errorFlags = 0;
}

void TpUartEmulator::handleDataOctet(uint16_t index, uint8_t data)
{
    if (index >= TPUART_MAX_FRAME_SIZE)
    {
        errorFlags |= TPUART_PROTOCOL_ERROR;
        return;
    }
    assembleBuffer[index] = data;
}

void TpUartEmulator::handleFrameEnd(uint16_t index, uint8_t data)
{
    txOffset = 0;

    if (index >= TPUART_MAX_FRAME_SIZE)
    {
        errorFlags |= TPUART_PROTOCOL_ERROR;
        queueByte(L_DATA_CON);
        return;
    }

    assembleBuffer[index] = data;
    submitFrame(index + 1);
}

void TpUartEmulator::submitFrame(uint16_t length)
{
    bool valid = (length >= LPDU_STD_OVERHEAD) && (length <= TPUART_MAX_FRAME_SIZE);

    // sblib's Bus state machine implements standard frames only.
    valid = valid && ((assembleBuffer[0] & LPDU_FRAME_TYPE_MASK) == LPDU_FRAME_TYPE_STD);
    valid = valid && (length == (uint16_t)((assembleBuffer[5] & 0x0F) + LPDU_STD_OVERHEAD));

    if (!valid)
    {
        errorFlags |= TPUART_PROTOCOL_ERROR;
        queueByte(L_DATA_CON);
        return;
    }

    if (txPending || stopMode)
    {
        // The host did not wait for the previous confirmation, or the
        // transceiver is detached from the bus.
        errorFlags |= TPUART_PROTOCOL_ERROR;
        queueByte(L_DATA_CON);
        return;
    }

    memcpy(txFrame, assembleBuffer, length);

    // Bus::prepareTelegram() overwrites the sender address with bcu->ownAddress().
    // A transceiver has to put the frame on the bus verbatim, so adopt the source
    // address of this frame first. This is what makes tunneling with several
    // individual addresses work.
    bcu.setOwnAddress(makeWord(txFrame[1], txFrame[2]));

    // The last octet is the checksum, sblib recalculates and appends it.
    bcu.bus->sendTelegram(txFrame, (unsigned short)(length - 1));
    txPending = true;
    txSuppressCon = false;
    txStartTime = millis();
}

void TpUartEmulator::pollHost()
{
    int data;
    while ((data = serial.read()) >= 0)
    {
        hostRxLedOffTime = millis() + LED_BLINK_MS;
        digitalWrite(LED_SERIAL_RX, LED_ON);
        processHostByte((uint8_t)data);
    }
}

/*
 * ---------------------------------------------------------------------------
 * KNX bus
 * ---------------------------------------------------------------------------
 */

void TpUartEmulator::pollKnxReceive()
{
    if (!bcu.bus->telegramReceived())
    {
        return;
    }

    uint16_t length = (uint16_t)bcu.bus->telegramLen;

    if ((length < LPDU_STD_OVERHEAD) || (length > TPUART_MAX_FRAME_SIZE))
    {
        errorFlags |= TPUART_RECEIVE_ERROR;
        bcu.bus->discardReceivedTelegram();
        return;
    }

    if (!queueFree(length))
    {
        // Apply back pressure: as long as the telegram is not discarded, sblib
        // reports the receive buffer as busy and stops acknowledging.
        return;
    }

    queueBytes(bcu.bus->telegram, length);
    bcu.bus->discardReceivedTelegram();

    knxRxLedOffTime = millis() + LED_BLINK_MS;
    digitalWrite(LED_KNX_RX, LED_ON);
}

void TpUartEmulator::pollKnxTransmit()
{
    if (!txPending)
    {
        return;
    }

    if (!bcu.bus->sendingFrame())
    {
        txPending = false;
        if (!txSuppressCon)
        {
            // sblib does not expose the transmission result, and it already
            // repeats a frame on NACK and BUSY. Report a positive confirmation.
            queueByte((uint8_t)(L_DATA_CON | L_DATA_CON_SUCCESS));
        }
        txSuppressCon = false;
        return;
    }

    if ((uint32_t)(millis() - txStartTime) >= TPUART_TX_CONFIRM_TIMEOUT_MS)
    {
        txPending = false;
        errorFlags |= TPUART_TRANSMIT_ERROR;
        if (!txSuppressCon)
        {
            queueByte(L_DATA_CON);
        }
        txSuppressCon = false;
    }
}

void TpUartEmulator::pollLeds()
{
    uint32_t now = millis();

    if ((int32_t)(now - knxRxLedOffTime) >= 0)
    {
        digitalWrite(LED_KNX_RX, LED_OFF);
    }
    if ((int32_t)(now - hostRxLedOffTime) >= 0)
    {
        digitalWrite(LED_SERIAL_RX, LED_OFF);
    }
}

/*
 * ---------------------------------------------------------------------------
 * Controller -> host, paced output queue
 * ---------------------------------------------------------------------------
 */

bool TpUartEmulator::queueFree(uint16_t count) const
{
    uint16_t used = (uint16_t)((txQueueTail - txQueueHead) & TX_QUEUE_MASK);
    return ((TPUART_TX_QUEUE_SIZE - 1 - used) >= count);
}

void TpUartEmulator::queueByte(uint8_t data)
{
    if (!queueFree(1))
    {
        errorFlags |= TPUART_PROTOCOL_ERROR;
        return;
    }
    txQueue[txQueueTail] = data;
    txQueueTail = (uint16_t)((txQueueTail + 1) & TX_QUEUE_MASK);
}

void TpUartEmulator::queueBytes(const uint8_t* data, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++)
    {
        queueByte(data[i]);
    }
}

void TpUartEmulator::drainQueue()
{
    while (txQueueHead != txQueueTail)
    {
#if (TPUART_TX_PACING_MS > 0)
        if ((uint32_t)(millis() - lastTxByteTime) < TPUART_TX_PACING_MS)
        {
            return;
        }
#endif
        if (serial.write(txQueue[txQueueHead]) != 1)
        {
            return; // serial write buffer is full, try again next round
        }
        txQueueHead = (uint16_t)((txQueueHead + 1) & TX_QUEUE_MASK);
        lastTxByteTime = millis();

#if (TPUART_TX_PACING_MS > 0)
        return; // one octet per pacing interval
#endif
    }
}
