/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#include "sb_timer.h"

static unsigned int ticks;

#define SINGLE_SHOT_PERIOD  1
#define LPC_TMR_MR3INT     (1 << 3)

inline unsigned int get_time(void)
{
    unsigned int result;

    result = ticks | LPC_TMR16B0->TC;
    // check if an overflow has happened since we read the timer
    if (LPC_TMR16B0->IR & LPC_TMR_MR3INT)
    {
        LPC_TMR16B0->IR = LPC_TMR_MR3INT;
        ticks          += 0x10000;
        // recalculate the result
        result          = ticks | LPC_TMR16B0->TC;
    }
    return result;
}

void sb_timer_init (void)
{
    // set the prescaler of the timer to create a 100µs tick
    LPC_TMR16B0->PR   = SystemCoreClock / 1000000 * 100;
    LPC_TMR16B0->TCR  = 0x02;
    LPC_TMR16B0->MR3  = 0x0000; // create an overflow event
    LPC_TMR16B0->CTCR = 0x0000;
    LPC_TMR16B0->TCR  = 0x01;
    ticks             = 0;
}

void sb_timer_start(SbTimer * timer, unsigned int offset, unsigned int period)
{
    if (!offset) offset = period;
    if (!period) period = SINGLE_SHOT_PERIOD;
    timer->period       = period;
    timer->timeout      = get_time() + offset;
}

unsigned int sb_timer_check (SbTimer * timer)
{
    unsigned int result = 0;
    int diff = get_time() - timer->timeout;

    if (timer->period && (diff > 0))
    {
        result = 1;
        if (timer->period != SINGLE_SHOT_PERIOD)
        {
            timer->timeout += timer->period;
        }
        else
            timer->period   = 0;
    }
    return result;
}
