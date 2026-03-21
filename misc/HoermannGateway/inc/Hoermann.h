/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef HOERMANN_H_
#define HOERMANN_H_

#include "CRC.h"
#include "HoermannState.h"
#include <sblib/timeout.h>
#include <sblib/types.h>


class Hoermann {
public:
    Hoermann() = default;

    /**
     * Initializes the serial interface for RS485 communication with the Hoermann drive.
     *
     * Configures the UART peripheral at 19200 baud (8N1), assigns the TX and RX pins,
     * sets up the RTS pin for hardware RS485 driver enable (active-low DE), and enables
     * RS485 direction control via the UART hardware.
     *
     * @param pinTx  Pin to use as UART transmit (TX).
     * @param pinRx  Pin to use as UART receive (RX), configured with pull-up and hysteresis.
     * @param pinRTS Pin to use as RS485 direction control (RTS / DE), driven low during transmission.
     */
    void begin(uint32_t pinTx, uint32_t pinRx, uint32_t pinRTS);

    void loop();
    void open();
    void close();
    void stop();
    void venting();
    void light(bool on);
    void emergencyStop(bool on);

    HoermannState state;

    void debugSendPeriodic();

protected:
    enum StateMachine : uint8_t
    {
        WAIT_FOR_BREAK = 0,
        WAIT_FOR_ADDRESS = 1,
        WAIT_FOR_COUNTER_AND_LENGTH = 2,
        WAIT_FOR_DATA = 3,
        WAIT_FOR_CRC = 4
    };
    uint8_t slaveResponse[2] = { 0x00, 0x10 };
    StateMachine stateMachine = WAIT_FOR_BREAK;
    int address = 0;
    int counter = 0;
    int length = 0;
    int position = 0;
    byte data[15] = {};
    uint8_t myCounter = 0;
    CRC crc;

    void sendResponse(uint8_t addr, uint8_t bytes[], uint8_t len);

private:
    // 18 = 1 + 1 + 15 + 1 (address + counter/length + data + crc)
    constexpr static uint8_t MAX_FRAME_LENGTH = 18;
    constexpr static uint16_t SERIAL_RX_TIMEOUT_MS = 35;
    uint32_t lastRxReceiveTick = 0;
    volatile bool isrBreakDetected = false;
    volatile uint32_t isrFrameErrorCount = 0;
    volatile uint32_t isrParityErrorCount = 0;
    volatile uint32_t isrOverrunCount = 0;
    uint8_t serialBuffer[MAX_FRAME_LENGTH] = {};
    int16_t bufferPosition = -1;

    char lastSend = 'a';
    constexpr static char SEPARATOR = ';';
    uint32_t lastSysTick = 0;

    Timeout serialRxTimeout;

    void onSerialError(uint32_t lineStatus);

    void resetSerialBuffer();
    void putSerialBuffer(uint8_t toPut);
    [[nodiscard]] uint8_t getSerialBufferSize() const;
    bool crcOKofSerialBuffer();
    void printSerialBuffer() const;
};

#endif /* HOERMANN_H_ */

