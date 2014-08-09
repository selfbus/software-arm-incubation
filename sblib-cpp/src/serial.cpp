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

    // Drop data from the RX FIFO
    while (LPC_UART->LSR & LSR_RDR)
        val = LPC_UART->RBR;

    NVIC_EnableIRQ(UART_IRQn);    // Enable UART interrupt
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
    return (LPC_UART->LSR & LSR_THRE) ? 1 : 0;
}

int Serial::read()
{
    if (peeked >= 0)
    {
        int ch = peeked;
        peeked = -1;
        return ch;
    }

    if (LPC_UART->LSR & LSR_RDR)
        return LPC_UART->RBR;

    return -1;
}

int Serial::peek()
{
    if (peeked < 0 && (LPC_UART->LSR & LSR_RDR))
        peeked = LPC_UART->RBR;

    return peeked;
}

extern "C" void UART_IRQHandler()
{
}
