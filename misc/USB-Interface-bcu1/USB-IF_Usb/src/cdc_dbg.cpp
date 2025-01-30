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

void CdcDbgIf::receiveAndPushToUart()
{
    if (!ReceiveEna)
    {
        return;
    }

    uint32_t bytesReadyCount = vcom_read_cnt();
    while (bytesReadyCount)
    {
        deviceIf.BlinkActivityLed();
        int buffno = buffmgr.AllocBuffer();
        if (buffno < 0)
        {
            // failHardInDebug();
            return; // Ohne verfügbaren Buffer bleibt nur der exit.
        }

        if (ser_txfifo.Full() == TFifoErr::Full)
        {
            //failHardInDebug();
            return; // uart tx fifo is full
        }

        uint8_t* ptr = buffmgr.buffptr(buffno);
        uint32_t bytesReadCount = vcom_bread(&ptr[cdc_OffSet], BUFF_SIZE - cdc_OffSet); ///\todo was 64, but can be 65
        if (bytesReadCount == 0)
        {
             // das ist merkwürdig...
             failHardInDebug();
             buffmgr.FreeBuffer(buffno);
             break;
        }
        ptr[0] = bytesReadCount + cdc_OffSet;
        ptr[2] = C_HRH_IdCdc; // Kennung fuer CDC-Paket

        TFifoErr pushResult = ser_txfifo.Push(buffno);
        if (pushResult != TFifoErr::Ok)
        {
            failHardInDebug();
            buffmgr.FreeBuffer(buffno);
        }

        rxByteCounter += bytesReadCount;
        ReceiveEna = true; // immer nur ein Paket vom CDC-Interface im Fifo erlaubt
        RecDisTime = systemTime;
        bytesReadyCount = vcom_read_cnt();
    }
}

void CdcDbgIf::DbgIf_Tasks(void)
{
    static int txcnt = 0;
    if (!USB_IsConfigured(hUsb))
    {
        return;
    }

    // receive (Rx) part of the CDC virtual com port
    if ((CdcDeviceMode == TCdcDeviceMode::ProgBusChip) || (CdcDeviceMode == TCdcDeviceMode::ProgUserChip))
    {
        // Nur in diesen Modi werden Daten vom CDC-VCOM Interface angenommen
        // und zum uart gesendet
        receiveAndPushToUart();
    }
    else
    {
        PurgeRx();
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

    // transmit (Tx) part of the CDC virtual com port
    if ((CdcDeviceMode == TCdcDeviceMode::ProgBusChip) || (CdcDeviceMode == TCdcDeviceMode::ProgUserChip) ||
        (CdcDeviceMode == TCdcDeviceMode::BusMon) || (CdcDeviceMode == TCdcDeviceMode::UsbMon))
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

    if (CdcDeviceMode == TCdcDeviceMode::ProgUserChip)
    {
        // Set soft uart control lines (RTS/CTS...) of the KNX mcu´s soft uart (prog_uart)
        uint8_t new_ctrl = vcom_readctrllines();
        if (new_ctrl != CtrlLines)
        {
            CtrlLines = new_ctrl;
        int buffno = buffmgr.AllocBuffer();
        if (buffno >= 0)
        {
            uint8_t *buffptr = buffmgr.buffptr(buffno);
            *buffptr++ = C_Dev_Packet_Length;
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
