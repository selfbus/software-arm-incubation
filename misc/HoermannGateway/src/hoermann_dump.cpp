/*
*  This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include "hoermann_dump.h"
#include "config.h"


/** Instance of the debugging software UART, exposed in hoermann_dump.h */
HoermannDump hoermannDump(PIO_SOFT_UART_RX, PIO_SOFT_UART_TX,
    timer16_0, SoftUART::BaudRate::Baud38400, SystemCoreClock);

HoermannDump::HoermannDump(const uint32_t rxPin, const uint32_t txPin,
    Timer& timer, const BaudRate baudRate, const uint32_t systemClock)
   :
    SoftUART(rxPin, txPin, timer, baudRate, systemClock),
    lastDump(0)
{
}

void HoermannDump::begin()
{
    SoftUART::begin();
    println("Selfbus garage door gateway");
    print("Tx/Rx");
    print(SEPARATOR);
    print("frame");
    print(SEPARATOR);
    print("count");
    print(SEPARATOR);
    print("delta ms");
    print(SEPARATOR);
    print("crc status");
    print(SEPARATOR);
    print("frame crc");
    print(SEPARATOR);
    print("calculated crc");
    println();
    updateLastDumpTime();
}

void HoermannDump::dumpRx(const bool crcIsOK, const uint8_t crc,
    const uint8_t* buffer, const int16_t bufferSize)
{
    print("Rx");
    print(SEPARATOR);
    for (uint16_t i = 0; i < bufferSize; i++)
    {
        print(buffer[i], HEX, 2);
        if (i != bufferSize - 1)
        {
            print(" ");
        }
    }
    print(SEPARATOR);
    print(bufferSize);
    print(SEPARATOR);
    print(getDumpTimeDelta());
    print(SEPARATOR);

    if (crcIsOK)
    {
        print("ok");
    }
    else
    {
        print("error");
    }
    print(SEPARATOR);
    print(buffer[bufferSize - 1], HEX, 2);
    print(SEPARATOR);
    print(crc, HEX, 2);
    println();
    updateLastDumpTime();
}

void HoermannDump::dumpTx(const uint8_t address, const uint8_t counterAndLength,
    const uint8_t crcTosend, const uint8_t * payload,
    const uint8_t payloadLength)
{
    print("Tx"); print(SEPARATOR);
    print(address, HEX, 2);
    print(" ", counterAndLength, HEX, 2);

    for (uint8_t i = 0; i < payloadLength; i++)
    {
        print(" ", payload[i], HEX, 2);
    }
    print(" ", crcTosend, HEX, 2);
    print(SEPARATOR);
    print(payloadLength + 3); // +1 address, +1 counter/length, +1 crc
    print(SEPARATOR);
    print(getDumpTimeDelta());
    print(SEPARATOR);
    print("ok");
    print(SEPARATOR);
    print(crcTosend, HEX, 2);
    print(SEPARATOR);
    print(crcTosend, HEX, 2);
    println();
    updateLastDumpTime();
}

void HoermannDump::updateLastDumpTime()
{
    lastDump = millis();
}

uint32_t HoermannDump::getDumpTimeDelta() const
{
    return millis() - lastDump;
}

// Replace default timer16_0 interrupt handler
extern "C" void TIMER16_0_IRQHandler()
{
    hoermannDump.timerInterruptHandler();
}
