/*
 *  Copyright (c) 2018 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef APP_RGB_SWITCH
#define APP_RGB_SWITCH

#include "params.h"
#include <sblib/eibBCU1.h>

extern BCU1 bcu;

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

/**
 * Handle the periodic function which are not triggered by a change of an input
 */
void handlePeriodic(void);

#endif /*APP_RGB_SWITCH*/
