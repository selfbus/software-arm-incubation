/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
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
 * Initialization of the timer module.
 *
 * Internally the internal 16 timer 0 hardware timer is used.
 */

void sb_timer_init(void);

/**
 * Start a new timer. To define a single shot timer use 0 as period.
 * If a cyclic timer will be started and the offset parameter is zero,
 * the period will be used as offset.
 *
 * @param timer   The timer object which should be started
 * @param offset  Number of 100µs ticks after which the first timer event
 *                should expire the first time.
 * @param period  Number of 100µs ticks defining the period of cyclic timer
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
