/*
 *  types.h - Data types
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_types_h
#define sblib_types_h

/**
 * An unsigned 1 byte value.
 */
typedef unsigned char byte;

/**
 * An unsigned 2 byte value.
 */
typedef unsigned short word;

/**
 * A boolean value that only uses 1 byte.
 */
typedef unsigned char boolean;

/**
 * Configuration modes.
 */

/**
 * Configuration modes for digital pins.
 */
enum PinConfigMode
{
    /**
     * Disable the function.
     */
    DISABLE = 0,

    /**
     * Generate an interrupt on match.
     */
    INTERRUPT = 1,

    /**
     * Reset the timer on match.
     */
    RESET = 2,

    /**
     * Stop the timer on match.
     */
    STOP = 4,

    /**
     * Clear the corresponding digital to 0 pin on match.
     */
    CLEAR = 0x10,

    /**
     * Set the corresponding digital pin to 1 on match.
     */
    SET = 0x20,

    /**
     * Toggle the corresponding digital pin on match.
     */
    TOGGLE = 0x30,

    /**
     * Capture on rising edge: a 0 followed by a 1 on the capture pin.
     */
    RISING_EDGE = 0x40,

    /**
     * Capture on falling edge: a 1 followed by a 0 on the capture pin.
     */
    FALLING_EDGE = 0x80
};

/**
 * Declare a function as always inline
 */
#define ALWAYS_INLINE __attribute__((always_inline)) inline

#endif /*sblib_types_h*/
