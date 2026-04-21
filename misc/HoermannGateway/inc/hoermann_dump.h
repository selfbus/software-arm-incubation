/*
*  This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#ifndef HOERMANNGATEWAY_HOERMANN_DUMP_H
#define HOERMANNGATEWAY_HOERMANN_DUMP_H

#include "sblib/soft_uart.h"

extern "C" void TIMER16_0_IRQHandler();
// Note: timer16_1 is reserved for the class Bus of the sblib
extern "C" void TIMER32_0_IRQHandler();
extern "C" void TIMER32_1_IRQHandler();

extern "C" void PIOINT0_IRQHandler();
extern "C" void PIOINT1_IRQHandler();
extern "C" void PIOINT2_IRQHandler();
extern "C" void PIOINT3_IRQHandler();

class HoermannDump : SoftUART
{
public:
    HoermannDump(uint32_t rxPin, uint32_t txPin, Timer& timer, BaudRate baudRate, uint32_t systemClock);

    HoermannDump() = delete;
    HoermannDump(const HoermannDump&) = delete;
    HoermannDump(HoermannDump&&) = delete;
    HoermannDump& operator=(const HoermannDump&) = delete;
    HoermannDump& operator=(HoermannDump&&) = delete;

    void begin() override;

    void dumpRx(bool crcIsOK, uint8_t crc, const uint8_t* buffer, int16_t bufferSize);
    void dumpTx(uint8_t address, uint8_t counterAndLength, uint8_t crcTosend,
        const uint8_t* payload, uint8_t payloadLength);

    using SoftUART::print;
    using SoftUART::println;

protected:
    friend void TIMER16_0_IRQHandler();
    // Note: timer16_1 is reserved for the class Bus of the sblib
    friend void TIMER32_0_IRQHandler();
    friend void TIMER32_1_IRQHandler();

    friend void PIOINT0_IRQHandler(); // For possible Rx Pin on Port 0
    friend void PIOINT1_IRQHandler(); // For possible Rx Pin on Port 1
    friend void PIOINT2_IRQHandler(); // For possible Rx Pin on Port 2
    friend void PIOINT3_IRQHandler(); // For possible Rx Pin on Port 3

private:
    constexpr static char SEPARATOR = ';';
    void updateLastDumpTime();
    [[nodiscard]] uint32_t getDumpTimeDelta() const;
    uint32_t lastDump;
};

/** Expose the debugging software UART instance declared in hoermann_dump.cpp */
extern HoermannDump hoermannDump;

#endif /* HOERMANNGATEWAY_HOERMANN_DUMP_H */
