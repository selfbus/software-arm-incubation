/*
 *  eib.h - Include the EIB related header files and define the
 *          objects that make a EIB device.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_eib_h
#define sblib_eib_h

#include <sblib/core.h>

#include <sblib/eib/bcu.h>
#include <sblib/eib/bus.h>
#include <sblib/eib/com_objects.h>

/**
 * The object for EIB bus access.
 *
 * The following resources are used:
 * - 16 bit timer #1,
 * - Pin PIO1_8 for receiving from the bus,
 * - Pin PIO1_10 for sending to the bus.
 */
extern Bus bus;

/**
 * Class for controlling all EIB related things.
 */
class EIB
{
public:
    /**
     * Begin using the EIB bus coupling unit.
     */
    void begin();

    /**
     * End using the EIB bus coupling unit.
     */
    void end();
};

#endif /*sblib_eib_h*/
