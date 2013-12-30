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

/**
 * The system time in usec since the last reset. The resolution of this time
 * depends on the wakeup time. This time is not available when the wakeup timer
 * is disabled.
 *
 * @see sb_set_wakeup_time()
 */
unsigned int sbSysTime;

#define SINGLE_SHOT_PERIOD  1

// Increment for sbSysTime on SysTick timeout
static unsigned int sbSysTickInc = -1;

/**
 * The system tick handler.
 */
void SysTick_Handler()
{
    sbSysTime += sbSysTickInc;
}

/**
 * Set the wakeup timer.
 *
 * @param timeout - the timeout in usec between wakeups. 0 to disable the timer.
 *
 * @brief This timer generates wakeup interrupts. It uses the SysTick timer, which
 * is a 24bit timer that runs with the system clock. The default timeout is 1msec.
 * Shorter timeouts give exacter sbSysTime. Longer timeouts use less resources.
 */
void sb_set_wakeup_time(unsigned int timeout)
{
    if(!timeout) timeout = 1000; // default wakeup timeout of 1ms

    sbSysTickInc = timeout;
    SysTick_Config(SystemCoreClock / 1000000 * timeout);
    NVIC_EnableIRQ(SysTick_IRQn);
}

void sb_timer_start(SbTimer * timer, unsigned int offset, unsigned int period)
{
    if (!offset) offset = period;
    if (!period) period = SINGLE_SHOT_PERIOD;
    timer->period       = period;
    timer->timeout      = sbSysTime + offset;
}

unsigned int sb_timer_check (SbTimer * timer)
{
    unsigned int result = 0;
    int diff = sbSysTime - timer->timeout;

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
