/*
 *  Selfbus-USB-IF.cpp - The application's main.
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "chip.h"
#include <stdio.h>
#include <string.h>
#include "app_usbd_cfg.h"
#include "nxp.h"
#include "hid_knx.h"
#include "cdc_vcom.h"
#include "busdevice_if.h"
#include "cdc_dbg.h"
#include "device_mgnt.h"
#include "device_mgnt_const.h"
#include "UartIf.h"
#include "ModeSelect.h"
#include "error_handler.h"


const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

extern "C" void SystemInit(void)
{
    deviceIf.SystemInit();
}

static USBD_HANDLE_T g_hUsb;

volatile unsigned alive_cnt;
bool usb_alive;


extern "C" void USB_IRQHandler(void)
{
  USBD_API->hw->EnableEvent(g_hUsb, 0, USB_EVT_SOF, 1);
	USBD_API->hw->ISR(g_hUsb);
}

extern "C" ErrorCode_t USB_sof_event(USBD_HANDLE_T hUsb)
{
  alive_cnt++;
  return LPC_OK;
}

int main(void) {
	unsigned Last10msTime = 0;
	usb_alive = false;

	CdcDeviceMode = TCdcDeviceMode::Halt;
	SystemCoreClockUpdate();

	deviceIf.PioInit();
	if (deviceIf.KnxSideProgMode())
	{
	  CdcDeviceMode = TCdcDeviceMode::ProgBusChip;
      modeSelect.SetLeds();
	} else	{ // wenn nicht "ProgBusChip"
	  modeSelect.StartModeSelect();
	  CdcDeviceMode = modeSelect.DeviceMode();
	}

	if (CdcDeviceMode == TCdcDeviceMode::ProgBusChip) // ISP enable for KNX module is set (JP5=on)
	{
	    uart.Init(9200, true); ///\todo check why 9200 and not at least 9600 isn't this the ISP-speed to the knx module?
	}
	else
	{
	    uart.Init(C_Dev_Baurate, false);
	}

	while (1) {
			devicemgnt.SysIf_Tasks(knxhidif.UsbIsConfigured());
			knxhidif.KnxIf_Tasks();
			if (CdcDeviceMode != TCdcDeviceMode::HidOnly) {
				cdcdbgif.DbgIf_Tasks();
			}

			if (uart.SerIf_Tasks())
				if (CdcDeviceMode == TCdcDeviceMode::ProgBusChip)
					cdcdbgif.reEnableReceive();

			if ((systemTime - Last10msTime) >= 10)
			{
			  Last10msTime = systemTime;
			  usb_alive = (alive_cnt != 0);
			  alive_cnt = 0;
			  deviceIf.DoActivityLed(usb_alive);
			  if (deviceIf.Hid2Knx_Ena()) // Folgendes nur, wenn nicht Prog-Bus-Chip aktiv ist
			  {
			    if (modeSelect.DoModeSelect())
			    {
			      bool restart = false;
			      if (CdcDeviceMode == TCdcDeviceMode::HidOnly)
			        restart = true;
			      CdcDeviceMode = modeSelect.DeviceMode();
			      if (CdcDeviceMode == TCdcDeviceMode::HidOnly)
			        restart = true;
			      if (restart)
			      {
			        USBD_API->hw->Connect(g_hUsb, 0);
			        while ((systemTime - Last10msTime) <= 500); // 0,5s warten, damit der PC das Disconnect bemerkt
			        NVIC_SystemReset();
			      }
			    }
			  }
			}
	}
    ErrorCode_t ret = usb_init(&g_hUsb, CdcDeviceMode == TCdcDeviceMode::HidOnly);
    if (ret != LPC_OK)
    {
        fatalError();
    }
}
