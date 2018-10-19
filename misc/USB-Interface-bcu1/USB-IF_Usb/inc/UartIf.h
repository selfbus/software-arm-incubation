/*
 *  UartIf.h - Uart interface for the inter-uC communication
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef UARTIF_H_
#define UARTIF_H_

// UART line status: receive data ready bit: RBR holds an unread character
#define LSR_RDR  0x01

// UART line status: overrun error bit
//#define LSR_OE   0x02

// UART line status: parity error bit
//#define LSR_PE   0x04

// UART line status: framing error bit
//#define LSR_FE   0x08

// UART line status: break interrupt bit
//#define LSR_BI   0x10

// UART line status: the transmitter hold register (THR) is empty
#define LSR_THRE 0x20

// UART line status: transmitter empty (THR and TSR are empty)
//#define LSR_TEMT 0x40

// UART line status: error in RX FIFO
//#define LSR_RXFE 0x80

// UART read-buffer-ready interrupt
#define UART_IE_RBR 0x01

// UART transmit-hold-register-empty interrupt
#define UART_IE_THRE 0x02

enum class TUartIfErr
{
	Ok,
	Error,
	Busy
};

class UartIf
{
public:
	void Init(int baudRate, bool rawMode);
	TUartIfErr TransmitBuffer(int buffno);
	bool TxBusy(void);
	void interruptHandler(void);
	bool SerIf_Tasks(void);
protected:
	uint8_t *txbuffptr;
	uint8_t *rxbuffptr;
	uint8_t chksum;
	bool rawmode;
	bool discard;
	int txlen;
	int rxlen;
	int txbuffno;
	int ser_buffno;
	int rxbuffno;
};

extern UartIf uart;

#endif /* UARTIF_H_ */
