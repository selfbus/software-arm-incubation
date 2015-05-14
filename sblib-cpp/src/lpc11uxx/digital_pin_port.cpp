/*
 *  digital_pin_port.cpp - Port configuration functions for digital I/O
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include <sblib/digital_pin.h>

#include <sblib/arrays.h>
#include <sblib/platform.h>
#include <sblib/utils.h>


void portMode(int portNum, int pinMask, int mode)
{
    LPC_GPIO_TypeDef* port = gpioPorts[portNum];
    const unsigned short type = mode & 0xf000;
    unsigned int iocon = mode & 0xfff;

    short func = (mode >> 18) & 31;
    if (!func) func = PF_PIO;

    if (type == OUTPUT || type == OUTPUT_MATCH)
    {
        port->DIR |= pinMask;
    }
    else // INPUT
    {
        port->DIR &= ~pinMask;
    }

    for (int pinNum = 0; pinMask; ++pinNum, pinMask >>= 1)
    {
        if (pinMask & 1)
            *(ioconPointer(portNum, pinNum)) = iocon;
    }
}

void portDirection(int portNum, int pinMask, int dir)
{
    LPC_GPIO_TypeDef* port = gpioPorts[portNum];

    if (dir == OUTPUT)
        port->DIR |= pinMask;
    else port->DIR &= ~pinMask;
}
