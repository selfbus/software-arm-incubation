/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef app_in8_h
#define app_in8_h

#include "com_objs.h"
#include <sblib/debounce.h>

/**
 * Number of input channels. Default: 8.
 * Can be set at compile time.
 */
#ifndef NUM_CHANNELS
#  define NUM_CHANNELS 8
#endif

extern const int inputPins[];
extern Debouncer inputDebouncer[NUM_CHANNELS];
/**
 * Calculate the time in ms based on a time base and a factor
 */
unsigned int calculateTime(int base, int factor);

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

/**
 * The values of the communication objects
 */
extern ObjectValues& objectValues;

#endif /*app_in8_h*/
