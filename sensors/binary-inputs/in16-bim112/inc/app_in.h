/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *                2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef app_in4_h
#define app_in4_h

#include <sblib/types.h>
#include <sblib/eibMASK0701.h>

extern MASK0701 bcu;

/**
 * A communication object was updated from somebody via the EIB bus.
 *
 * @param objno - the ID of the communication object.
 */
void objectUpdated(int objno);

/**
 * The value of an input channel changed.
 *
 * @param channel - the input channel (0..)
 * @param value - the new value
 */
void inputChanged(int channel, int value);

/**
 * Handle the periodic functions.
 */
void checkPeriodic(void);

/**
 * Initialization of the application
 */
void initApplication(void);

#endif /*app_in4_h*/
