/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sb_utils.h"

#include "internal/sb_hal.h"
#include "sb_timer.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#include "gpio.h"


/**
 * Initialize a debounce structure. It is sufficient to call this method
 * once, e.g. in the program's init() function.
 *
 * @param debounce - the debounce structure.
 * @param value - the value to set as the initial debounced value.
 */
void sb_init_debounce(SbDebounce* debounce, unsigned int value)
{
    debounce->value = value;
    debounce->time = 0;
}

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
unsigned int sb_debounce(unsigned int current, unsigned int timeout, SbDebounce* debounce)
{
    if (debounce->last != current || !debounce->time || sbSysTime < debounce->time)
    {
        debounce->time = sbSysTime;
        debounce->last = current;
    }
    else if (debounce->last == current && sbSysTime >= debounce->time + timeout)
    {
        debounce->time = 0;
        debounce->value = current;
    }

    return debounce->value;
}

/**
 * Indicates a fatal error and stops program execution.
 * The prog led will flash fast to indicate the error.
 */
void sb_fatal()
{
    unsigned int mask;

    LPC_GPIO[SB_PROG_PORT]->DIR |= 1 << SB_PROG_BIT; // Set prog button+led to output
    SysTick_Config(0xffffffUL);

    while (1)
    {
        if (SysTick->VAL & 0x800000)
            mask = SB_PROG_MASK;
        else mask = 0;

        LPC_GPIO[SB_PROG_PORT]->MASKED_ACCESS[SB_PROG_MASK] = mask;
    }
}
