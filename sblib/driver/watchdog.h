/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#ifndef WATCHDOG_H_
#define WATCHDOG_H_

/**
 * Initialize the watchdog.
 *
 * The clock of the watchdog will be initialized to 1 MHz.
 *
 * @param timeout    watchdog timeout in µs (max is 8s.388608)
 */
void WD_Init(unsigned int timeout);

/**
 * Trigger the watchdog.
 */
void WD_Trigger(void);

#endif /* WATCHDOG_H_ */
