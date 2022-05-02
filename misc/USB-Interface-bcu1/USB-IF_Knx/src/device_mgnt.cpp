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
#include "Emi_Knx.h"
#include "GenFifo.h"
#include "BufferMgr.h"
#include "device_mgnt.h"

DeviceManagement devicemgnt;

DeviceManagement::DeviceManagement()
{
  txtimeout = 0;
  rxtimeout = 0;
  LastDevSys = 0;
}

void DeviceManagement::DevMgnt_Tasks(void)
{
  if (dev_rxfifo.Empty() != TFifoErr::Empty)
  {
    int buffno;
    dev_rxfifo.Pop(buffno);
    uint8_t *ptr = buffmgr.buffptr(buffno);
    unsigned DevPacketLength = ptr[0];
    if ((ptr[2+A_HRH_Id] == C_HRH_IdDev) && (DevPacketLength == (2+3)))
    {
      rxtimeout = millis() + C_RxTimeout; // Wird bei jedem Paket an dieses If gesetzt.
      switch (ptr[2+A_HRH_Id+1])
      {
      case C_Dev_Sys:
        switch (ptr[2+A_HRH_Id+2])
        {
        case C_DevSys_Normal:
          if (LastDevSys != C_DevSys_Normal)
          {
            emiknxif.SetCdcMonMode(false);
            proguart.Disable();
            LastDevSys = C_DevSys_Normal;
          }
          break;
        case C_DevSys_Disable:
          /* - Usb side meldet USB unconfigured
          *   -> löscht CdcMonActive & HidIfActive
          *      löscht ProgUserChipModus
          */
          if (LastDevSys != C_DevSys_Disable)
          {
            emiknxif.RstSysState();
            emiknxif.SetCdcMonMode(false);
            proguart.Disable();
            LastDevSys = C_DevSys_Disable;
          }
          break;
        case C_DevSys_CdcMon:
          /* - Usb side aktiviert Cdc-Monitor
          *   -> setzt CdcMonActive
          *      löscht ProgUserChipModus
          */
          if (LastDevSys != C_DevSys_CdcMon)
          {
            emiknxif.SetCdcMonMode(true);
            proguart.Disable();
            LastDevSys = C_DevSys_CdcMon;
          }
          break;
        case C_DevSys_UsrPrg:
          /* - Usb side aktiviert Prog User Chip
          *   -> Initialisiert Interface, aktiviert den ProgUserChip Modus
          *      löscht CdcMonActive
          */
          if (LastDevSys != C_DevSys_UsrPrg)
          {
            emiknxif.SetCdcMonMode(false);
            proguart.Enable();
            LastDevSys = C_DevSys_UsrPrg;
          }
          break;
        }
        break;
      case C_Dev_Isp:
        /* - Usb side schickt Isp-En, -Reset Daten für das Isp-If
        *   -> weiterleiten zum entsprechenden If
        */
        proguart.SetIspLines(ptr[2+A_HRH_Id+2]);
        break;
      }
    }
    buffmgr.FreeBuffer(buffno);
  }

  if (((int)(millis() - rxtimeout) > 0) && (LastDevSys != 0))
  {
    /* Timeout, anscheinen ist die USB-Seite nicht funktionsfähig
    *   -> löscht CdcMonActive & HidIfActive
    *      löscht ProgUserChipModus
    */
    emiknxif.RstSysState();
    emiknxif.SetCdcMonMode(false);
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
      *buffptr++ = 0x05;
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
