/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "watchdog.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#define WDEN              (0x1 << 0)
#define WDRESET           (0x1 << 1)
#define WDTOF             (0x1 << 2)
#define WDINT             (0x1 << 3)
#define WDPROTECT         (0x1 << 4)


void WD_Init(unsigned int timeout)
{
    // Enable the clock to the watchdog peripheral
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<15);

    // setup the watchdog clock as 1 MHz
    LPC_SYSCON->WDTOSCCTRL = 0xC << 4 | 0x01; // 4 MHZ divided by 4
    LPC_SYSCON->PDRUNCFG  &= ~(0x1<<6);       // power up the watchdog oscillator

    LPC_SYSCON->WDTCLKSEL  = 0x02;            // Select watchdog oscillator
    LPC_SYSCON->WDTCLKUEN  = 0x01;            // Update clock
    LPC_SYSCON->WDTCLKUEN  = 0x00;            // Toggle update register once
    LPC_SYSCON->WDTCLKUEN  = 0x01;

    while (!(LPC_SYSCON->WDTCLKUEN & 0x01))   // Wait until updated
        ;

    LPC_SYSCON->WDTCLKDIV = 1;      // Divided by 1

    LPC_WDT->TC   = timeout;
    LPC_WDT->MOD  = WDEN | WDRESET; // Once WDEN is set, the WDT will start after feeding
    LPC_WDT->FEED = 0xAA;           // Feeding sequence
    LPC_WDT->FEED = 0x55;
}

void WD_Trigger()
{
    LPC_WDT->FEED = 0xAA;
    LPC_WDT->FEED = 0x55;
}

void WD_Stop()
{
    LPC_WDT->MOD &= ~WDEN;
}

void WD_Start()
{
    LPC_WDT->MOD |= WDEN;
}
