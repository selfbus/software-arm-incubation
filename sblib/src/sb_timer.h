/*
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#ifndef SB_TIMER_H_
#define SB_TIMER_H_

// data structure used for a timer object
typedef struct
{
    unsigned int  timeout;
    unsigned int  period;
} SbTimer;

/**
 * The system time in usec since the last reset. The resolution of this time
 * depends on the wakeup time. This time is not available when the wakeup timer
 * is disabled.
 *
 * @see sb_set_wakeup_time()
 */
extern unsigned int sbSysTime;

/**
 * Set the wakeup timer.
 *
 * @param timeout - the timeout in usec between wakeups.
 *
 * @brief This timer generates wakeup interrupts. It uses the SysTick timer, which
 * is a 24bit timer that runs with the system clock. The default timeout is 1msec.
 * Shorter timeouts give exacter sbSysTime. Longer timeouts use less resources.
 */
void sb_set_wakeup_time(unsigned int timeout);

/**
 * Start a new timer. To define a single shot timer use 0 as period.
 * If a cyclic timer will be started and the offset parameter is zero,
 * the period will be used as offset.
 *
 * @param timer   The timer object which should be started
 * @param offset  Microseconds after which the timer should expire for
 *                the first time.
 * @param period  Period of cyclic timer in microseconds
 */
void sb_timer_start(SbTimer * timer, unsigned int offset, unsigned int period);

/**
 * Check if a the event of the timer object has been expired.
 *
 * @param timer  The timer object which should be checked.
 * @return       1 .. The timer has expired
 *               0 .. The timer has no expired
 */
unsigned int sb_timer_check(SbTimer * timer);

#endif /* SB_TIMER_H_ */
