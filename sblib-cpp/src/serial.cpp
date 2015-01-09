/*
 *  serial.cpp - Serial port access.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/serial.h>

#include <sblib/digital_pin.h>
#include <sblib/platform.h>


// UART line status: receive data ready bit: RBR holds an unread character
#define LSR_RDR  0x01

// UART line status: overrun error bit
#define LSR_OE   0x02

// UART line status: parity error bit
#define LSR_PE   0x04

// UART line status: framing error bit
#define LSR_FE   0x08

// UART line status: break interrupt bit
#define LSR_BI   0x10

// UART line status: the transmitter hold register (THR) is empty
#define LSR_THRE 0x20

// UART line status: transmitter empty (THR and TSR are empty)
#define LSR_TEMT 0x40

// UART line status: error in RX FIFO
#define LSR_RXFE 0x80


Serial serial(PIO1_6, PIO1_7);


Serial::Serial(int rxPin, int txPin)
{
    pinMode(rxPin, SERIAL_RXD);
    pinMode(txPin, SERIAL_TXD);
}

void Serial::begin(int baudRate)
{
    begin(baudRate, SERIAL_8N1);
}

void Serial::begin(int baudRate, SerialConfig config)
{
    NVIC_DisableIRQ(UART_IRQn);

    LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 12; // Enable UART clock
    LPC_SYSCON->UARTCLKDIV = 1;           // divided by 1

    LPC_UART->LCR = 0x83;         // 8 bits, no parity, 1 stop bit

    unsigned int val = SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV /
        LPC_SYSCON->UARTCLKDIV / 16 / baudRate;

    LPC_UART->DLM  = val / 256;
    LPC_UART->DLL  = val % 256;

    LPC_UART->LCR = (int) config; // Configure data bits, parity, stop bits
    LPC_UART->FCR = 0x07;         // Enable and reset TX and RX FIFO.
    LPC_UART->MCR = 0;            // Disable modem controls (DTR, DSR, RTS, CTS)

    // Ensure a clean start, no data in either TX or RX FIFO
    flush();
    peeked = -1;
    rxBuffer = 0;

    // Drop data from the RX FIFO
    while (LPC_UART->LSR & LSR_RDR)
        val = LPC_UART->RBR;

    NVIC_DisableIRQ(UART_IRQn);    // Disable UART interrupt
}

void Serial::end()
{
    NVIC_DisableIRQ(UART_IRQn);              // Disable UART interrupt
    LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << 12); // Disable UART clock
}

int Serial::write(byte ch)
{
    // wait until the transmitter hold register is free
    while (!(LPC_UART->LSR & LSR_THRE))
        ;

    LPC_UART->THR = ch;
    return 1;
}

void Serial::flush()
{
    while ((LPC_UART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT))
        ;
}

int Serial::available()
{
    if (! rxBuffer)
        return LPC_UART->LSR & LSR_RDR ? 1 : 0;
    return rxBuffer->count;
}

ALWAYS_INLINE int Serial::_getByte(void)
{
    int result = -1;
    if (! rxBuffer)
    {
        if (LPC_UART->LSR & LSR_RDR)
            result = LPC_UART->RBR;
    }
    else if (rxBuffer->count)
    {
        result = rxBuffer->buffer[rxBuffer->tail++];
        rxBuffer->count--;
        rxBuffer->tail &= rxBuffer->size;
    }
    return result;
}

int Serial::read()
{
    if (peeked >= 0)
    {
        int ch = peeked;
        peeked = -1;
        return ch;
    }

    return _getByte();
}

int Serial::peek()
{
    if (peeked < 0 && (LPC_UART->LSR & LSR_RDR))
        peeked = _getByte();

    return peeked;
}

void Serial::setupReceiveRingBuffer(RingBuffer * buffer, SerialTriggerLevelConfig rxThreshold)
{
    rxBuffer = buffer;
    rxBuffer->head = rxBuffer->tail = rxBuffer->count = 0;
    LPC_UART->IER |= (1 << 0); // XXX
    NVIC_EnableIRQ(UART_IRQn);
}

void Serial::emptyRxFifo(void)
{
    while (LPC_UART->LSR & LSR_RDR)
    {
        rxBuffer->buffer[rxBuffer->head] = LPC_UART->RBR;
        if (rxBuffer->count <= rxBuffer->size)
        {
            rxBuffer->head++;
            rxBuffer->count++;
            rxBuffer->head &= rxBuffer->size;
        }
    }
}

extern "C" void UART_IRQHandler()
{
    serial.emptyRxFifo();
}
