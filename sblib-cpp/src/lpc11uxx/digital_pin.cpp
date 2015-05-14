/*
 *  digital_pin.cpp - Functions for digital I/O
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include <sblib/digital_pin.h>
#include <sblib/platform.h>
#include <sblib/utils.h>

void pinMode(int pin, int mode)
{
    unsigned int iocon = mode & 0xfff;

    short func = (mode >> 18) & 31;
    if (!func) func = PF_PIO;
    if (mode == INPUT_CAPTURE) func = PF_CAP;
    if (mode == OUTPUT_MATCH)  func = PF_MAT;
    if ((mode == OUTPUT_MATCH) || (mode == OUTPUT))
         pinDirection(pin, OUTPUT);
    else pinDirection(pin, INPUT);
    if ((pin & PFL_ADMODE) && func != PF_AD)
        iocon |= 0x80;

    if (func)
    {
        int funcNum = getPinFunctionNumber(pin, func);
        if (funcNum >= 0) iocon |= funcNum;
        else fatalError(); // the pin does not have the desired function
    }

    *(ioconPointer(pin)) = iocon;
}

void pinDirection(int pin, int direction)
{
    volatile long unsigned int * dir = & LPC_GPIO->DIR[digitalPinToPort(pin)];
    unsigned int mask = digitalPinToBitMask(pin);

    if (direction == OUTPUT)
        * dir  |= mask;
    else * dir &= ~mask;
}

#if 0
void pinInterruptMode(int pin, int mode)
{
    LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin)];
    unsigned short mask = digitalPinToBitMask(pin);

    /* Configure the pin as input */
    pinMode(pin, INPUT);

    /* Set the level/edge configuration */
    if (mode & 0x100) port->IS  |=  mask;
    else              port->IS  &= ~mask;

    /* Set the both edge configuration */
    if (mode & 0x010) port->IBE |=  mask;
    else              port->IBE &= ~mask;

    /* Set the edge/level type configuration */
    if (mode & 0x100) port->IEV |=  mask;
    else              port->IEV &= ~mask;

    /* Enable the ionterrupt for this pin */
    if (mode & 0x100) port->IE  |=  mask;
    else              port->IE  &= ~mask;
}
#endif
