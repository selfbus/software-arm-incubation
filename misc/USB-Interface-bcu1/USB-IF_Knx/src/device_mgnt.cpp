/*
 *  device_mgnt.cpp - Device management, mode management
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <stdio.h>
#include "prog_uart.h"
#include "knxusb_const.h"
#include "emi_knx.h"
#include "GenFifo.h"
#include "BufferMgr.h"
#include "device_mgnt.h"
#include "device_mgnt_const.h"
#include "error_handler.h"

DeviceManagement devicemgnt;

DeviceManagement::DeviceManagement()
{
    txtimeout = 0;
    rxtimeout = 0;
    LastDevSys = 0;
}

void DeviceManagement::handleDev_Sys(uint8_t subControlByte)
{
    if (LastDevSys == subControlByte)
    {
        // Mode didn´t change, so everything is already fine
        return;
    }
    LastDevSys = subControlByte;
    switch (LastDevSys)
    {
        // KNX-Interface or
        // USB-Monitor
        case C_DevSys_Normal:
            emiknxif.SetCdcMonMode(false);
            proguart.Disable();
            break;

        // USB disabled
        case C_DevSys_Disable:
            // - Usb side meldet USB unconfigured
            //   -> löscht CdcMonActive
            //      löscht ProgUserChipModus
            emiknxif.reset(); // Set bcu in download mode to disable all KNX bus communication
            proguart.Disable();
            break;

        // KNX-Monitor
        case C_DevSys_CdcMon:
            // - Usb side aktiviert Cdc-Monitor
            //   -> setzt CdcMonActive
            //      löscht ProgUserChipModus
            emiknxif.SetCdcMonMode(true);
            proguart.Disable();
            break;

        // Prog-Interface
        case C_DevSys_UsrPrg:
            // - Usb side aktiviert Prog User Chip
            //   -> Initialisiert Interface, aktiviert den ProgUserChip Modus
            //      löscht CdcMonActive
            emiknxif.reset(); // Set bcu in download mode to disable all KNX bus communication
            proguart.Enable();
            break;

        // should never happen
        default:
            failHardInDebug();
            break;
    }
}

void DeviceManagement::DevMgnt_Tasks(void)
{
    if (dev_rxfifo.Empty() != TFifoErr::Empty)
    {
        int buffno;
        dev_rxfifo.Pop(buffno);
        uint8_t *ptr = buffmgr.buffptr(buffno);
        uint8_t DevPacketLength = ptr[0];
        if ((ptr[2+IDX_HRH_Id] == C_HRH_IdDev) && (DevPacketLength == C_Dev_Packet_Length))
        {
            rxtimeout = millis() + C_RxTimeout; // Wird bei jedem Paket an dieses If gesetzt.
            uint8_t command = ptr[2+IDX_HRH_Id+1];
            uint8_t subCommand = ptr[2+IDX_HRH_Id+2];
            switch (command)
            {
                case C_Dev_Sys:
                    handleDev_Sys(subCommand);
                    break;

                case C_Dev_Isp:
                    // - Usb side schickt Isp-Enable, Isp-Reset Daten für das Isp-If
                    //   -> weiterleiten zum entsprechenden If
                    //
                    proguart.SetIspLines(subCommand);
                    break;

                default:
                    // should never happen, but
                    // happened with ptr[0..4] == 05 f6 03 01 00 (command == 1 (C_Dev_Idle), subCommand == 0)
                    // caused by an echo of our own sent C_Dev_Idle
                    // now with PULL_UP and HYSTERESIS on serial rx pin, this should really never happen
                    failHardInDebug();
                    break;
            }
        }
        buffmgr.FreeBuffer(buffno);
    }

    if (((int)(millis() - rxtimeout) > 0) && (LastDevSys != 0))
    {
        // Timeout, anscheinen ist die USB-Seite nicht funktionsfähig
        //   -> löscht CdcMonActive
        //      löscht ProgUserChipModus
        emiknxif.reset(); // Set bcu in download mode to disable all KNX bus communication
        proguart.Disable();
        LastDevSys = 0;
    }

    if ((int)(millis() - txtimeout) > 0)
    {
        // Idle Paket Richtung USB-Seite verschicken
        int buffno = buffmgr.AllocBuffer();
        if (buffno >= 0)
        {
            uint8_t *buffptr = buffmgr.buffptr(buffno);
            *buffptr++ = C_Dev_Packet_Length;
            buffptr++;
            *buffptr++ = C_HRH_IdDev;
            *buffptr++ = C_Dev_Idle;
            *buffptr++ = LastDevSys;
            if (ser_txfifo.Push(buffno) != TFifoErr::Ok)
                buffmgr.FreeBuffer(buffno);
        }
        txtimeout = millis() + C_IdlePeriod;
    }
}
