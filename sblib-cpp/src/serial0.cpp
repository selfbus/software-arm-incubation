/*
 *  serial.cpp - Serial port access. This file defines the default serial port.
 *
 *  Copyright (c) 2015 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/serial.h>
#include <sblib/digital_pin.h>

#if defined (__LPC11XX__)
Serial serial(PIO1_6, PIO1_7);
#elif defined (__LPC11UXX__)
Serial serial(PIO0_18, PIO0_19);
#endif

extern "C" void UART_IRQHandler()
{
    serial.interruptHandler();
}
