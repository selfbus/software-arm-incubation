/*
 *  UartIf.cpp - Uart interface for the inter-uC communication
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <GenFifo.h>
#include <stdio.h>
#include "BufferMgr.h"
#include "knxusb_const.h"
#include "UartIf.h"
#include "device_mgnt_const.h"
#include "error_handler.h"

UartIf uart;


#ifndef __USE_LPCOPEN
#include <sblib/digital_pin.h>
#include <sblib/interrupt.h>
#include <sblib/platform.h>

#define LPC_USART LPC_UART

extern "C" void UART_IRQHandler(void)
{
	uart.interruptHandler();
}

void UartIf::Init(int baudRate, bool rawMode, uint32_t pinTx, uint32_t pinRx)
{
	disableInterrupt(UART_IRQn);
	txbuffno = -1;
	ser_buffno = -1;
	rxbuffno = -1;
	txlen = 0;
	discard = false;
	rawmode = rawMode;
	// Uart konfigurieren
    pinMode(pinRx, SERIAL_RXD);
    pinMode(pinTx, SERIAL_TXD);

	LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 12; // Enable UART clock
	LPC_SYSCON->UARTCLKDIV = 1;           // divided by 1

	LPC_UART->LCR = 0x80 | 3;

	unsigned int val = SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV /
			LPC_SYSCON->UARTCLKDIV / 16 / baudRate;

	LPC_UART->DLM  = val / 256;
	LPC_UART->DLL  = val % 256;

	LPC_UART->LCR = 3;             // Configure 8N1
	LPC_UART->FCR = 0x87;          // Enable and reset TX and RX FIFO, RxInt Level 8
	LPC_UART->MCR = 0;             // Disable modem controls (DTR, DSR, RTS, CTS)
	LPC_UART->IER |= UART_IE_RBR;  // Enable RX/TX interrupts

	// Drop data from the RX FIFO
	while (LPC_UART->LSR & LSR_RDR)
		val = LPC_UART->RBR;

	enableInterrupt(UART_IRQn);
}

#define NVIC_EnableIRQ enableInterrupt
#define NVIC_DisableIRQ disableInterrupt
#define UART0_IRQn UART_IRQn

#else
#include "chip.h"
// Handler und Init einfach getrennt schreiben je nach Target, dass ist zu aufwändig einzeln über defines...
extern "C" void UART_IRQHandler(void)
{
	uart.interruptHandler();
}

void UartIf::Init(int baudRate, bool rawMode)
{
	NVIC_DisableIRQ(UART0_IRQn);
	txbuffno = -1;
	ser_buffno = -1;
	rxbuffno = -1;
	txlen = 0;
	discard = false;
	rawmode = rawMode;
	// configure Uart (IOCON_FUNC3) with hysteresis and pull-up
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 13, IOCON_FUNC3 | IOCON_HYS_EN | IOCON_MODE_PULLUP); // PIO1_13 as TxD
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 14, IOCON_FUNC3 | IOCON_HYS_EN | IOCON_MODE_PULLUP); // PIO1_14 as RxD

	LPC_SYSCTL->SYSAHBCLKCTRL |= 1 << 12; // Enable UART clock
	LPC_SYSCTL->USARTCLKDIV = 1;           // divided by 1

	LPC_USART->LCR = 0x80 | 3;

	unsigned int val = SystemCoreClock * LPC_SYSCTL->SYSAHBCLKDIV /
			LPC_SYSCTL->USARTCLKDIV / 16 / baudRate;

	LPC_USART->DLM  = val / 256;
	LPC_USART->DLL  = val % 256;

	LPC_USART->LCR = 3;             // Configure 8N1
	LPC_USART->FCR = 0x87;          // Enable and reset TX and RX FIFO, RxInt Level 8
	LPC_USART->MCR = 0;             // Disable modem controls (DTR, DSR, RTS, CTS)
	LPC_USART->IER |= UART_IE_RBR;  // Enable RX/TX interrupts

	// Drop data from the RX FIFO
	while (LPC_USART->LSR & LSR_RDR)
		val = LPC_USART->RBR;

	NVIC_EnableIRQ(UART0_IRQn);
}
#endif

void UartIf::interruptHandler(void)
{
	// Der Transmitter ist vergleichsweise simpel, er versendet das übergebene Paket unverändert und ohne
	// Interpretation. Ist er mit dem Paket fertig, wird der Speicherplatz nicht freigegeben.
	if ((LPC_USART->IER & UART_IE_THRE) && (LPC_USART->LSR & LSR_THRE))
	{
		if (txlen == 0)
		{
			txbuffno = -1;
			LPC_USART->IER &= ~UART_IE_THRE;
		}
		else
		{
			// Wenn das Transmit Holding Reg leer ist, sind im Fifo Platz für 16 Wörter
			int cnt = 16;
			if (txlen < 16)
			{
				cnt = txlen;
			}
			txlen -= cnt;
			while (cnt-- > 0)
			{
				LPC_USART->THR = *txbuffptr++;
			}
		}
	}
	// Die Empfangsseite ist wesentlich aufwändiger. Sie empfängt Daten, besorgt sich bei Bedarf einen Buffer und
	// reiht ein vollständig empfangenes Paket in den passenden Fifo zum CDC oder HID Interface ein.
	// Gleichzeitig werden zwei unterschiedliche Modi unterstützt: Einen "RawMode", bei dem alle ankommenden
	// Bytes einfach an das CDC-Interface weitergeleitet werden (Programmiermodus für den KNX-Side Controller).
	// Und andererseits einen paketorientierten Modus, an dem Pakete mit Längenbyte und Checksumme empfangen werden.
	bool rxtimeout = false;
	if ((LPC_USART->IIR & 0xE) == 0xC) // Character timeout
	{
		//failHardInDebug(); ///\todo have seen this, enable to debug
	    rxtimeout = true; // das kann aber nicht die einzige Paket-Ende Erkennung sein, ansonsten wäre die Wartezeit verpflichtend!
	}
	if (LPC_USART->LSR & LSR_RDR)
	{
		//bool repeat = false;
		//while (repeat) Kein Repeat, lieber die ISR noch einmal aufrufen
		{
			if (rawmode)
			{
				//  Nicht paketorientiert:
				//  Egal ob Timeout Trigger oder Fifo Level Trigger: Verfügbare Bytes aus dem Fifo lesen, ein Paket draus machen,
				//  in den Fifo zum CDC-Interface einreihen.
				if (rxbuffno < 0)
					rxbuffno = buffmgr.AllocBuffer();
				if (rxbuffno >= 0)
				{
					rxbuffptr = buffmgr.buffptr(rxbuffno);
					rxlen = 0;
					uint8_t* ptr = rxbuffptr+3;
					while ((LPC_USART->LSR & LSR_RDR) && (rxlen < 64))
					{
						*ptr++ = LPC_USART->RBR;
						rxlen++;
					}
					*rxbuffptr++ = rxlen+3;
					*rxbuffptr++ = 0; // Checksumme wird nicht mitgeführt in diesem Modus
					*rxbuffptr = C_HRH_IdCdc; // Das Paket als CDC-Paket kennzeichnen
					if (cdc_txfifo.Push(rxbuffno) != TFifoErr::Ok)
					  buffmgr.FreeBuffer(rxbuffno);
					rxbuffno = -1;
				} else { // Receiver leeren, hilft ja nix
					while (LPC_USART->LSR & LSR_RDR)
					{
						LPC_USART->RBR;
					}
				}
			} else {
				// Paketorientiert:
				if (discard)
				{
					if (rxtimeout)
					{
						while (LPC_USART->LSR & LSR_RDR)
							LPC_USART->RBR; // if the readBuffer ist full, empty UART
						discard = false;
					} else {
						// Da bei einem festen Füllstand getriggert wird, können jetzt TriggerLevel-1 Bytes entnommen
						// werden - und der Fifo ist nicht leer. Notwendig damit der Timeout Interrupt später ausgelöst wird.
						for (int i=0; i<7; i++)
							LPC_USART->RBR;
					}
				} else {
					if (rxbuffno < 0)
					{
						// Kein Buffer aktiv? Buffer besorgen, öffnen. Im ersten Byte steht die Länge drin, damit ist die auch
						// schon bekannt.
						uint8_t len = LPC_USART->RBR;
						if ((len < 2) || (len > 67)) // das Längenbyte und die Checksumme werden mitgezählt
						{
							// Länge unplausibel? In einen "discard mode" wechseln, in dem alle Bytes bis zu einem Timeout verworfen werden.
							// Dafür müsste bei einem Fifo-Level trigger immer ein Byte im Fifo verbleiben, der Timeout erfolgt sonst nie.
							discard = true; // damit stimmt die ursprüngliche Länge im Fifo nicht mehr und
							//repeat = true; // der discard Algo macht evtl Schwierigkeiten
						} else {
							rxbuffno = buffmgr.AllocBuffer();
							if (rxbuffno < 0)
							{
								discard = true;
							} else {
								rxbuffptr = buffmgr.buffptr(rxbuffno);
								chksum = len;
								*rxbuffptr++ = len--;
								rxlen = len;
							}
						}
					}
					if (!discard)
					{
						while ((LPC_USART->LSR & LSR_RDR) && (rxbuffno >= 0))
						{
							//  Bytes aus dem Fifo lesen, in den Buffer schreiben.
							//  Alle Bytes empfangen? Checksumme testen. Passt nicht? Dann Paket verwerfen. Noch Bytes im Fifo? In den Discard-Mode wechseln.
							//  Paket beendet? Prüfen, wer das Ziel ist und Buffer nummer in den entsprechenden Fifo einreihen.
							*rxbuffptr = LPC_USART->RBR;
							chksum += *rxbuffptr++;
							rxlen--;
							if (rxlen == 0)
							{
								if (chksum == 0xff) // Die Summe über alles muss 0xFF ergeben
								{
									rxbuffptr = buffmgr.buffptr(rxbuffno);
									TFifoErr err = TFifoErr::Error;
									if (rxbuffptr[2] == C_HRH_IdHid) // An dieser Stelle stände die HID Report Nummer - und die muss 1 sein
										err = hid_txfifo.Push(rxbuffno);
									else if (rxbuffptr[2] == C_HRH_IdCdc) // Daten für die CDC-Schnittstelle
										err = cdc_txfifo.Push(rxbuffno);
									else if (rxbuffptr[2] == C_HRH_IdDev) // Daten für die interne Verwaltung
									  err = dev_rxfifo.Push(rxbuffno);
									if (err != TFifoErr::Ok)
										buffmgr.FreeBuffer(rxbuffno); // komisches Paket oder Fifo-Fehler -> weg damit
								} else { // Vorerst wird nur das Paket verworfen bei einem Empfangsfehler
									buffmgr.FreeBuffer(rxbuffno);
									if (LPC_USART->LSR & LSR_RDR)
										discard = true;
								}
								rxbuffno = -1;
							}
						}
					}
				}
			}
		}
	}
}

bool UartIf::TxBusy(void)
{
	return (txbuffno >= 0);
}

TUartIfErr UartIf::TransmitBuffer(int buffno)
{
	if (TxBusy())
	{
		return TUartIfErr::Busy;
	}

	uint8_t *ptr = buffmgr.buffptr(buffno);
    uint8_t len = *ptr;
    if ((len < 2) || (len > 67)) // auch im RawMode haben die Pakete den 2 Byte Header mit Längenangabe
    {
        failHardInDebug();
        return TUartIfErr::Error;
    }

    uint8_t acc = len;
    if (!rawmode)
    {
        // Berechnen der Checksumme
        len -= 2;
        ptr += 2;
        while (len--)
        {
            acc += *ptr++;
        }
        acc = 255-acc;
    }
    NVIC_DisableIRQ(UART0_IRQn);
    txbuffptr = buffmgr.buffptr(buffno);
    txlen = *txbuffptr;
    if (rawmode)
    {
        txbuffptr += cdc_OffSet; // Längenangabe, Checksumme und CDC-Id wird im Raw-Mode nicht mitgesendet
        txlen -= cdc_OffSet;
    } else {
        *(txbuffptr+1) = acc; // set calculated checksum
    }
    txbuffno = buffno;
    LPC_USART->IER |= UART_IE_THRE;
    NVIC_EnableIRQ(UART0_IRQn);
    NVIC_SetPendingIRQ(UART0_IRQn);
    //LPC_USART->THR = 0x55; // dummy

	return TUartIfErr::Ok;
}

// Rückgabewert true, wenn ein Cdc-Uart Paket verschickt worden ist
bool UartIf::SerIf_Tasks(void)
{
	if (TxBusy())
	{
	    return false;
	}

	if (ser_buffno >= 0)
    {
        ///\todo Der Versand des letzten Pakets wurde wohl gerade beendet
        /// Hier koennten noch irgendwelche Checks gemacht werden. Erneutes Versenden
        /// bei einem Fehler o.ä.
        buffmgr.FreeBuffer(ser_buffno);
        ser_buffno = -1;
    }

    if (ser_txfifo.Empty() == TFifoErr::Empty)
    {
        return false;
    }

    bool retval = false;
    ser_txfifo.Pop(ser_buffno);
    uint8_t *ptr = buffmgr.buffptr(ser_buffno);
    if (ptr[2] == C_HRH_IdCdc)
    {
        retval = true;
    }

    TUartIfErr err = TransmitBuffer(ser_buffno);
    if (err != TUartIfErr::Ok)
    {
        ///\todo Momentan als einzige Fehlerbehandlung: Paket verwerfen
        failHardInDebug();
        buffmgr.FreeBuffer(ser_buffno);
        ser_buffno = -1;
    }
	return retval;
}
