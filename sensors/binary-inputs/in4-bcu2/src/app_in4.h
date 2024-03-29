/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef app_in4_h
#define app_in4_h

#include <sblib/eibBCU2.h>
#include "com_objs.h"

extern BCU2 bcu;

/**
 * Number of input channels. Default: 4.
 * Can be set at compile time.
 */
#ifndef NUM_CHANNELS
#  define NUM_CHANNELS 4
#endif

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
 * Restore the state of the objects after a restart of the CPU
 */
void initApplication(void);

#endif /*app_in4_h*/
