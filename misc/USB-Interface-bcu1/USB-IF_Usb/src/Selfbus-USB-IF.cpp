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

static USBD_HANDLE_T g_hUsb;
volatile uint32_t usbHeartbeatCounter;

extern "C" void SystemInit(void)
{
    deviceIf.SystemInit();
}

extern "C" void USB_IRQHandler(void)
{
    USBD_API->hw->EnableEvent(g_hUsb, 0, USB_EVT_SOF, 1);
    USBD_API->hw->EnableEvent(g_hUsb, 0, USB_EVT_DEV_ERROR, 1);

    USBD_API->hw->ISR(g_hUsb);
}

extern "C" ErrorCode_t USB_sof_event(USBD_HANDLE_T hUsb) // Start of Frame event
{
    usbHeartbeatCounter++;
    return LPC_OK;
}

/**
 * USB_Error_Event handler
 *
 * At the moment it´s here just for debugging
 */
extern "C" ErrorCode_t USB_Error_Event(USBD_HANDLE_T hUsb, uint32_t param)
{
    // ERR_USBD_UNHANDLED
    return LPC_OK;
}

/**
 * USB_Configure_Event handler
 *
 * At the moment it´s here just for debugging
 */
extern "C" ErrorCode_t USB_Configure_Event(USBD_HANDLE_T hUsb)
{
    // ERR_USBD_UNHANDLED
    return LPC_OK;
}

///\todo add more USB Handler
/// check out usb_param.USB_... in usb_init(..)  (nxp/nxp.c) for more info
/// There´s also the enum USBD_EVENT_T for even more USB events

void systemReset()
{
    USBD_API->hw->Connect(g_hUsb, 0);
    uint32_t lastSysTick = systemTime;
    while ((systemTime - lastSysTick) <= 500)
    {
        ; // 0,5s warten, damit der PC das Disconnect bemerkt
    }
    NVIC_SystemReset();
}

int main(void) {
    currentDeviceMode = DeviceMode::Halt;
    SystemCoreClockUpdate();

    deviceIf.PioInit();
    if (deviceIf.KnxSideProgMode())
    {
        // jumper JP 5 is closed (PIO 1_19 connected to ground)
        currentDeviceMode = DeviceMode::ProgBusChip;
        modeSelect.setAllLeds(true);
        uart.Init(C_Dev_Baurate, true);
    }
    else
    {
        // wenn nicht "ProgBusChip"
        modeSelect.StartModeSelect();
        currentDeviceMode = modeSelect.getDeviceMode();
        uart.Init(C_Dev_Baurate, false);
    }

    ErrorCode_t ret = usb_init(&g_hUsb, currentDeviceMode == DeviceMode::HidOnly);
    if (ret != LPC_OK)
    {
        fatalError();
    }

    uint32_t last10msTime = 0;
    while (1)
    {
        devicemgnt.SysIf_Tasks(knxhidif.UsbIsConfigured());
        knxhidif.KnxIf_Tasks();
        if (currentDeviceMode != DeviceMode::HidOnly)
        {
            cdcdbgif.DbgIf_Tasks(); // virtual serial port (cdc = communication class device, dbgif = debugInterface ?)
        }

        if (uart.SerIf_Tasks())
        {
            if (currentDeviceMode == DeviceMode::ProgBusChip)
            {
                cdcdbgif.reEnableReceive();
            }
        }


        // Every 10ms checks
        if ((systemTime - last10msTime) < 10)
        {
            continue;
        }

        // below code runs only every 10ms
        last10msTime = systemTime;
        bool usb_alive = (usbHeartbeatCounter != 0);
        usbHeartbeatCounter = 0;
        deviceIf.DoActivityLed(usb_alive);

        if (!deviceIf.Hid2Knx_Ena())
        {
            continue;
        }

        // Folgendes nur, wenn nicht Prog-Bus-Chip aktiv ist
        if (modeSelect.DoModeSelect())
        {
            if (currentDeviceMode == DeviceMode::HidOnly)
                systemReset();

            currentDeviceMode = modeSelect.getDeviceMode();
            if (currentDeviceMode == DeviceMode::HidOnly)
                systemReset();
        }
    } // while (1)
}
