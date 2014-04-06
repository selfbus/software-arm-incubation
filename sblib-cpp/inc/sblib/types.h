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
 * Bit order
 */
enum BitOrder
{
    /**
     * Bit order: least significant bit first.
     */
    LSBFIRST = 1,

    /**
     * Bit order: most significant bit first.
     */
    MSBFIRST
};

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
 * IDs of the timers.
 */
enum TimerID
{
    /**
     * ID of the 16 bit timer #0
     */
    TIMER16_0,

    /**
     * ID of the 16 bit timer #1
     */
    TIMER16_1,

    /**
     * ID of the 32 bit timer #0
     */
    TIMER32_0,

    /**
     * ID of the 32 bit timer #1
     */
    TIMER32_1

};


/**
 * IDs of the timer match channels.
 */
enum TimerMatch
{
    /**
     * ID of the timer match channel #0
     */
    MAT0,

    /**
     * ID of the timer match channel #1
     */
    MAT1,

    /**
     * ID of the timer match channel #2
     */
    MAT2,

    /**
     * ID of the timer match channel #3
     */
    MAT3
};


/**
 * IDs of the timer capture channels.
 */
enum TimerCapture
{
    /**
     * ID of the timer capture channel #0
     */
    CAP0,

    /**
     * ID of the timer capture channel #1
     */
    CAP1
};


/**
 * IDs of the timer PWM channels.
 */
enum TimerPWM
{
    /**
     * ID of the timer PWM channel #0.
     */
    PWM0,

    /**
     * ID of the timer PWM channel #1.
     */
    PWM1,

    /**
     * ID of the timer PWM channel #2.
     */
    PWM2,

    /**
     * ID of the timer PWM channel #3.
     */
    PWM3
};


/**
 * Declare a function as always inline
 */
#define ALWAYS_INLINE __attribute__((always_inline)) inline

#endif /*sblib_types_h*/
