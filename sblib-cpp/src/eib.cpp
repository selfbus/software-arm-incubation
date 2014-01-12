/*
 *  eib.cpp - Assemble the EIB components.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib.h>


// The default bus access object
Bus bus(timer16_1);

// The interrupt handler for the default bus access object
BUS_TIMER_INTERRUPT_HANDLER(TIMER16_1_IRQHandler, bus);


void EIB::begin()
{
    bus.begin();
}

void EIB::end()
{
    bus.end();
}
