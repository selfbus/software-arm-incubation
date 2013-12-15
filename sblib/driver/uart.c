/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#include "uart.h"

#define IER_RBR     0x01
#define IER_THRE    0x02
#define IER_RLS     0x04

#define IIR_PEND    0x01
#define IIR_RLS     0x03
#define IIR_RDA     0x02
#define IIR_CTI     0x06
#define IIR_THRE    0x01

#define LSR_RDR     0x01
#define LSR_OE      0x02
#define LSR_PE      0x04
#define LSR_FE      0x08
#define LSR_BI      0x10
#define LSR_THRE    0x20
#define LSR_TEMT    0x40
#define LSR_RXFE    0x80

#ifdef UART_RX_BUFFER

#define BUFFER_SIZE 32
typedef struct
{
    unsigned char head;
    unsigned char tail;
    unsigned char count;
    unsigned char buffer[BUFFER_SIZE];
} UART_Buffer;

static UART_Buffer _rxb;

#endif

void UART_Init (unsigned int baud_rate)
{
    unsigned int div;
    unsigned int temp;

    // configure the pins used by the UART
    LPC_IOCON->PIO1_6 = 0x0001; // UART RXD
    LPC_IOCON->PIO1_7 = 0x0001; // UART TXD

    // Enable UART clock
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
    LPC_SYSCON->UARTCLKDIV     = 0x1;     // divided by 1

    LPC_UART->LCR = 0x80  // enable the DLAB
                  | 0x00  // no parity
                  | 0x00  // 1 stop bit
                  | 0x03; // 8 bit words
    temp          = LPC_SYSCON->UARTCLKDIV;

    div = (((SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV) / temp) / 16) / baud_rate;

    LPC_UART->DLM  = div / 256;
    LPC_UART->DLL  = div % 256;
    LPC_UART->LCR &= 0x3F;    // clear the DLAB bit
    // TODO play with receive threshold to reduce interrupt load
    LPC_UART->FCR  = 0x07;    // Enable and reset TX and RX FIFO

    // Read to clear the line status.
    temp = LPC_UART->LSR;

    /* Ensure a clean start, no data in either TX or RX FIFO. */
    while (( LPC_UART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT) );
    while ( LPC_UART->LSR & LSR_RDR )
    {
        temp = LPC_UART->RBR;   // Dump data from RX FIFO
    }
#ifdef UART_RX_BUFFER
    _rxb.head = _rxb.tail = _rxb.count = 0;
    LPC_UART->IER = IER_RBR;    // Enable UART receive interrupt
    NVIC_EnableIRQ(UART_IRQn);
#endif
}

void UART_PutChar (char byte)
{
    while (! (LPC_UART->LSR & LSR_THRE)); // wait until we can write a byte to the FIFO
    LPC_UART->THR = byte;
}

void UART_PutString (char * string, signed short length)
{
    if (length == -1)
    {
        for (length = 0; string [length] != 0; length++);
    }
    while (length--)
    {
        while (! (LPC_UART->LSR & LSR_THRE)); // wait until we can write a byte to the FIFO
        LPC_UART->THR = * string++;
    }
}

unsigned char UART_GetChar (void)
{
    unsigned char result;
#ifdef UART_RX_BUFFER
    if (_rxb.count)
    {
        _rxb.count--;
        result = _rxb.buffer [_rxb.tail++];
        if (_rxb.tail > BUFFER_SIZE)
            _rxb.tail = 0;
    }
    else
    {
        while (! (LPC_UART->LSR & LSR_RDR)); // wait until a byte is in the RX fifo
        result = LPC_UART->RBR;
    }
    // TODO implement the ring buffer handling
#else
    while (! (LPC_UART->LSR & LSR_RDR)); // wait until a byte is in the RX fifo
    result = LPC_UART->RBR;
#endif
    return result;
}

#ifdef UART_RX_BUFFER
void UART_IRQHandler (void)
{
    while (LPC_UART->LSR & LSR_RDR)
    {
        _rxb.buffer [_rxb.head++] = LPC_UART->RBR;
        _rxb.count++;
        if (_rxb.head > BUFFER_SIZE)
            _rxb.head = 0;
    }
}
#endif
