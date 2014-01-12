/*
 *  bus.cpp - Low level EIB bus access.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/bus.h>

#include <sblib/core.h>


// The default bus access object
Bus bus(timer16_1);

// The interrupt handler for the default bus access object
BUS_TIMER_INTERRUPT_HANDLER(TIMER16_1_IRQHandler, bus);


Bus::Bus(Timer& aTimer)
:timer(aTimer)
{
}

void Bus::begin()
{
    timer.begin();
}

void Bus::end()
{
    timer.end();
}
