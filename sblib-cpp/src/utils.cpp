/*
 *  utils.cpp - Utility functions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/utils.h>

#include <sblib/digital_pin.h>
#include <sblib/platform.h>


void reverseCopy(byte* dest, const byte* src, int len)
{
    src += len - 1;
    while (len > 0)
    {
        *dest++ = *src--;
        --len;
    }
}

void fatalError()
{
    // We use only low level functions here as a fatalError() could happen
    // anywhere in the lib and we want to ensure that the function works

    int mask = digitalPinToBitMask(PIO0_7);
    LPC_IOCON->PIO0_7 = 0xd0;
    LPC_GPIO0->DIR |= mask;

    SysTick_Config(0x1000000);

    while (1)
    {
        // Blink the LED on the LPCxpresso board
        LPC_GPIO0->MASKED_ACCESS[mask] = (SysTick->VAL & 0x800000) == 0 ? mask : 0;
    }
}
