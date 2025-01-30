/*
 *  cdc_dbg.cpp - CDC USB serial emulator: Sending and receiving in buffers
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "chip.h"
#include "cdc_vcom.h"
#include "busdevice_if.h"
#include "BufferMgr.h"
#include "GenFifo.h"
#include "knxusb_const.h"
#include "device_mgnt_const.h"
#include "cdc_dbg.h"
#include "error_handler.h"

CdcDbgIf cdcdbgif;

CdcDbgIf::CdcDbgIf() :
        hUsb(nullptr),
        ReceiveEna(true),
        zlp(false),
        CtrlLines(0xff),
        RecDisTime(0),
        rxByteCounter(0),
        txByteCounter(0)
{
}

void CdcDbgIf::Set_hUsb(USBD_HANDLE_T h_Usb)
{
	hUsb = h_Usb;
	ReceiveEna = true;
	zlp = false;
	CtrlLines = 0xff;
}

void CdcIfSet_hUsb(USBD_HANDLE_T h_Usb)
{
  cdcdbgif.Set_hUsb(h_Usb);
}


void CdcDbgIf::reEnableReceive(void)
{
	ReceiveEna = true;
}

void CdcDbgIf::PurgeRx(void)
{
	while (vcom_read_cnt())
	{
		uint8_t buff[64];
		vcom_bread(buff, 64);
	}
}

void CdcDbgIf::DbgIf_Tasks(void)
{
	static int txcnt = 0;
	if ( USB_IsConfigured(hUsb))
	{
		if ((CdcDeviceMode == TCdcDeviceMode::ProgBusChip) || (CdcDeviceMode == TCdcDeviceMode::ProgUserChip))
		{ // Nur in diesen Modi werden Daten vom CDC-VCOM Interface angenommen
			if (ReceiveEna)
			{
				while (vcom_read_cnt())
				{
				  deviceIf.BlinkActivityLed();
					int buffno = buffmgr.AllocBuffer();
					if (buffno < 0)
						return; // Ohne verfügbaren Buffer bleibt nur der exit.
					uint8_t* ptr = buffmgr.buffptr(buffno);
					unsigned rdCnt = vcom_bread(&ptr[3], 64);
					if (rdCnt == 0)
					{ // das ist merkwürdig...
						buffmgr.FreeBuffer(buffno);
						break;
					}
					ptr[0] = rdCnt+3;
					ptr[2] = 2; // Kennung fuer CDC-Paket
		    	if (ser_txfifo.Push(buffno) != TFifoErr::Ok)
		    		buffmgr.FreeBuffer(buffno);
					ReceiveEna = false; // immer nur ein Paket vom CDC-Interface im Fifo erlaubt
					RecDisTime = systemTime;
				}
			}
		} else {
			PurgeRx();
		}
	}

	// Normalerweise wird ReceiveEna wieder gesetzt, wenn die Übertragung bestätigt wurde.
	// Für den Fall, dass z.B. unerwarteterweise die KNX-Seite inaktiv ist, muss eine
	// dauerhafte Blockierung vermieden werden. Daher gibt es zusätzlich einen Timeout
	// länger als das längste denkbare Paket (64 Bytes bei 9600 Baud).
	if (!ReceiveEna && ((systemTime - RecDisTime) > CDC2UARTMAXWAIT))
	{
	    failHardInDebug();
		ReceiveEna = true;
	}

	if ( USB_IsConfigured(hUsb) &&
			((CdcDeviceMode == TCdcDeviceMode::ProgBusChip) || (CdcDeviceMode == TCdcDeviceMode::ProgUserChip) ||
					(CdcDeviceMode == TCdcDeviceMode::BusMon) || (CdcDeviceMode == TCdcDeviceMode::UsbMon)))
	{
		if (vcom_txbusy() == LPC_OK)
		{
			if (cdc_txfifo.Empty() != TFifoErr::Empty)
			{
				int buffno;
				cdc_txfifo.Pop(buffno);
				uint8_t *ptr = buffmgr.buffptr(buffno);
				zlp = (ptr[0]-3) == 64;
				vcom_write(&ptr[3], ptr[0]-3);
				buffmgr.FreeBuffer(buffno);
				if (++txcnt == 2)
				{
					txcnt=0;
				}
        deviceIf.BlinkActivityLed();
			} else {
				if (zlp)
				{
					zlp = false;
					uint8_t dummy;
					vcom_write(&dummy, 0);
				}
			}
		}
	} else {
		int buffno;
		while (cdc_txfifo.Empty() != TFifoErr::Empty)
		{
			cdc_txfifo.Pop(buffno);
			buffmgr.FreeBuffer(buffno);
		}
	}

	if ( USB_IsConfigured(hUsb) && (CdcDeviceMode == TCdcDeviceMode::ProgUserChip))
	{
		uint8_t new_ctrl = vcom_readctrllines();
		if (new_ctrl != CtrlLines)
		{
			CtrlLines = new_ctrl;
	    int buffno = buffmgr.AllocBuffer();
	    if (buffno >= 0)
	    {
	    	uint8_t *buffptr = buffmgr.buffptr(buffno);
	    	*buffptr++ = 0x05;
	    	buffptr++;
	    	*buffptr++ = C_HRH_IdDev;
	    	*buffptr++ = C_Dev_Isp;
    		*buffptr++ = CtrlLines;
	    	if (ser_txfifo.Push(buffno) != TFifoErr::Ok)
	    	{
	    	    failHardInDebug();
	    	    buffmgr.FreeBuffer(buffno);
	    	}
	    }
      deviceIf.BlinkActivityLed();
		}
	} else {
		CtrlLines = 0xff;
		// Die KNX-Seite wird über das Device Management informiert, wenn sich der Modus ändert.
	}
}
