/*
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef sb_utils_h
#define sb_utils_h

// Constant for debouncing 10 msec
#define SB_DEBOUNCE_10MS    10000

// Constant for debouncing 100 msec
#define SB_DEBOUNCE_100MS   100000


/**
 * The debounce data structure.
 */
typedef struct
{
    unsigned int value;  // The currently valid value
    unsigned int last;   // The value of the last sb_debounce() call
    unsigned int time;   // The system time when the value last changed
} SbDebounce;

/**
 * Initialize a debounce structure. It is sufficient to call this method
 * once, e.g. in the program's init() function.
 *
 * @param debounce - the debounce structure.
 * @param value - the value to set as the initial debounced value.
 */
void sb_init_debounce(SbDebounce* debounce, unsigned int value);

/**
 * Debounce an integer value. The debounced value is returned until a new
 * value stays the same for all sb_debounce() calls within the timeout duration.
 *
 * @param current - the current value.
 * @param timeout - the time to wait unti the value becomes valid, in usec.
 * @param debounce - the debounce structure.
 *
 * @return The debounced value.
 */
unsigned int sb_debounce(unsigned int current, unsigned int timeout, SbDebounce* debounce);

/**
 * Indicates a fatal error and stops program execution.
 * The prog led will flash fast to indicate the error.
 */
void sb_fatal();

/**
 * Calculate the offset of a member in a structure.
 *
 * @param type - the type or structure
 * @param field - the field within the type
 *
 * @brief E.g. for the structure
 *        struct foo
 *        {
 *            short a;
 *            short b;
 *        };
 *        SB_OFFSET_OF(foo, b) returns 2
 */
#define SB_OFFSET_OF(type, field) ((unsigned long) &(((type *) 0)->field))

#endif /*sb_utils_h*/
