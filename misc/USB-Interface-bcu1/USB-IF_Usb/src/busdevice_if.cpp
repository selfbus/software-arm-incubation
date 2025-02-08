/*
 *  busdevice_if.cpp - Pin configuration, system timer, activity LED
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "chip.h"
#include "usbd_rom_api.h"
#include "nxp.h"
#include "busdevice_if.h"

volatile unsigned int systemTime;

DeviceMode currentDeviceMode;

DeviceIf deviceIf;

#define ACTLED_HPRD 10

DeviceIf::DeviceIf()
{
    enabled = true; // Damit die LED beim Start AUSgeschaltet wird
}

void DeviceIf::SetActivityLed(bool onoff)
{
    Chip_GPIO_SetPinState(LPC_GPIO, 1, 28, !onoff);
}

void DeviceIf::BlinkActivityLed(void)
{
    if (!enabled)
    {
        return;
    }

    if (blinkcnt >= 2)
    {
        return;
    }

    if (blinkcnt != 0)
    {
        return;
    }
    SetActivityLed(false);
    blinkcnt += 2;
    timecnt = ACTLED_HPRD;
}

void DeviceIf::DoActivityLed(bool LedEnabled)
{
    if (enabled != LedEnabled)
    {
        enabled = LedEnabled;
        SetActivityLed(enabled);
    }
    if (enabled)
    {
        if (blinkcnt != 0)
        {
            if (timecnt > 0)
                timecnt--;
            if (timecnt == 0)
            {
                blinkcnt--;
                if (blinkcnt > 0)
                {
                    timecnt = ACTLED_HPRD;
                    SetActivityLed((blinkcnt & 1) != 0);
                }
            }
        }
    } else {
        blinkcnt = 0;
        timecnt = 0;
    }
}

bool DeviceIf::Hid2Knx_Ena(void)
{
    if (currentDeviceMode == DeviceMode::HidOnly)
        return true;
    if (currentDeviceMode == DeviceMode::BusMon)
        return true;
    if (currentDeviceMode == DeviceMode::UsbMon)
        return true;
    if (currentDeviceMode == DeviceMode::ProgUserChip)
        return true;
    return false;
}

void DeviceIf::SystemInit(void)
{
    SystemSetupClocking();
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);
}

void DeviceIf::PioInit(void)
{
    Chip_GPIO_Init(LPC_GPIO);
    ///\todo check why is PIO0_3 configured here? It is not used by our code, but the mcu´s bootloader evaluates it on reset
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0,  3, IOCON_FUNC1 | IOCON_MODE_INACT),  // PIO0_3 used for USB_VBUS
    //Chip_IOCON_PinMuxSet(LPC_IOCON, 0,  6, IOCON_FUNC1 | IOCON_MODE_INACT),  // PIO0_6 used for USB_CONNECT
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 11, IOCON_FUNC1 | IOCON_MODE_PULLUP); // JTAG Pins, unused
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 12, IOCON_FUNC1 | IOCON_MODE_PULLUP); //   on the board. But
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 13, IOCON_FUNC1 | IOCON_MODE_PULLUP); //   config them as GPIO
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 14, IOCON_FUNC1 | IOCON_MODE_PULLUP); //   to avoid noise sensitivity
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 6, IOCON_FUNC1);                      // GPIO0_6: Function as USB_CONNECT

    // PIO0_3 and PIO1_0 are evaluates by the mcu´s bootloader on reset (see UM10462 20.4 page 390)
    // If PIO0_3 is high the bootloader enters the ISP in USB mode (connector X3)
    // If PIO0_3 is low the bootloader enters the ISP in UART mode (connector P2)
    ///\todo check why is PIO0_3 configured here? It is not used by our code, but the mcu´s bootloader evaluates it on reset
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 3, IOCON_FUNC0 | IOCON_MODE_PULLUP);  // ISP UART Enable
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 19, IOCON_FUNC0 | IOCON_MODE_PULLUP); // KNX Side ISP Enable
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 25, IOCON_FUNC0 | IOCON_MODE_PULLUP); // Switch input

    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 28); // PIO1_28 as "(USB) Activity" LED

    Chip_Clock_SetUSBClockSource(SYSCTL_USBCLKSRC_PLLOUT, 1);
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_USB);
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_USBRAM);
    Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_USBPAD_PD);

    // Configure the system timer to call SysTick_Handler once every 1 msec
    SysTick_Config(SystemCoreClock / 1000);
    systemTime = 0;
}

bool DeviceIf::KnxSideProgMode(void)
{
    return Chip_GPIO_GetPinState(LPC_GPIO, 1, 19) == false;
}

extern "C" void SysTick_Handler()
{
    ++systemTime;
}
