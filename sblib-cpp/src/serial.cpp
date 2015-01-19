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
#include <sblib/interrupt.h>
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

// UART read-buffer-ready interrupt
#define UART_IE_RBR 0x01

// UART transmit-hold-register-empty interrupt
#define UART_IE_THRE 0x02


Serial::Serial(int rxPin, int txPin)
{
    pinMode(rxPin, SERIAL_RXD);
    pinMode(txPin, SERIAL_TXD);
}

void Serial::begin(int baudRate, SerialConfig config)
{
    disableInterrupt(UART_IRQn);

    LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 12; // Enable UART clock
    LPC_SYSCON->UARTCLKDIV = 1;           // divided by 1

    LPC_UART->LCR = 0x80 | config;

    unsigned int val = SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV /
        LPC_SYSCON->UARTCLKDIV / 16 / baudRate;

    LPC_UART->DLM  = val / 256;
    LPC_UART->DLL  = val % 256;

    LPC_UART->LCR = (int) config;  // Configure data bits, parity, stop bits
    LPC_UART->FCR = 0x07;          // Enable and reset TX and RX FIFO.
    LPC_UART->MCR = 0;             // Disable modem controls (DTR, DSR, RTS, CTS)
    LPC_UART->IER |= UART_IE_RBR;  // Enable RX/TX interrupts

    // Ensure a clean start, no data in either TX or RX FIFO
    flush();
    clearBuffers();

    // Drop data from the RX FIFO
    while (LPC_UART->LSR & LSR_RDR)
        val = LPC_UART->RBR;

    enableInterrupt(UART_IRQn);
}

void Serial::end()
{
    flush();
    disableInterrupt(UART_IRQn);
    LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << 12); // Disable UART clock
}

int Serial::write(byte ch)
{
#ifdef SERIAL_WRITE_DIRECT

    // wait until the transmitter hold register is free
   while (!(LPC_UART->LSR & LSR_THRE))
       ;
   LPC_UART->THR = ch;
   return 1;

#else

    if (writeHead == writeTail && (LPC_UART->LSR & LSR_THRE))
    {
        // Transmitter hold register and write buffer are empty -> directly send
        LPC_UART->THR = ch;
        LPC_UART->IER |= UART_IE_THRE;
        return 1;
    }

    int writeTailNext = (writeTail + 1) & BufferedStream::BUFFER_SIZE_MASK;

    // Wait until the output buffer has space
    while (writeHead == writeTailNext)
        ;

    writeBuffer[writeTail] = ch;
    writeTail = writeTailNext;
    LPC_UART->IER |= UART_IE_THRE;

    return 1;

#endif
}

void Serial::flush()
{
    while ((LPC_UART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT))
        ;
}

int Serial::read()
{
    bool readFull = readBufferFull();
    int ch = BufferedStream::read();

    if (readFull && (LPC_UART->LSR & LSR_RDR))
    {
        disableInterrupt(UART_IRQn);
        interruptHandler();
        enableInterrupt(UART_IRQn);
    }

    return ch;
}

void Serial::interruptHandler()
{
    if (LPC_UART->LSR & LSR_THRE)
    {
        if (writeHead == writeTail)
        {
            LPC_UART->IER &= ~UART_IE_THRE;
        }
        else
        {
            LPC_UART->THR = writeBuffer[writeHead];

            ++writeHead;
            writeHead &= BufferedStream::BUFFER_SIZE_MASK;
        }
    }

    while ((LPC_UART->LSR & LSR_RDR) && !readBufferFull())
    {
        readBuffer[readTail] = LPC_UART->RBR;

        ++readTail;
        readTail &= BufferedStream::BUFFER_SIZE_MASK;
    }
}
