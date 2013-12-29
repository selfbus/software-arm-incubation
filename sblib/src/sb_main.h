/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sb_main_h
#define sb_main_h

/**
 * Initialize the library. Call this function once when the program starts.
 */
void sb_init();

/**
 * The lib's main processing. Call this function regularily from your main().
 * It should be called at least every 100ms.
 */
void sb_main_loop();

/**
 * Set manufacturer data, manufacturer-ID, and device type.
 *
 * @param data - the manufacturer data
 * @param manufacturer - the manufacturer ID
 * @param deviceType - the device type
 * @param version - the version of the application program
 */
void sb_set_appdata(unsigned short data, unsigned short manufacturer, unsigned short deviceType,
                    unsigned char version);
/**
 * The system time in usec since the last reset. The resolution of this time
 * depends on the wakeup time. This time is not available when the wakeup timer
 * is disabled.
 *
 * @see sb_set_wakeup_time()
 */
unsigned int sbSysTime;

/**
 * Set the wakeup timer.
 *
 * @param timeout - the timeout in usec between wakeups. 0 to disable the timer.
 *
 * @brief This timer generates wakeup interrupts. It uses the SysTick timer, which
 * is a 24bit timer that runs with the system clock. The default timeout is 1msec.
 * Shorter timeouts give exacter sbSysTime. Longer timeouts use less resources.
 */
void sb_set_wakeup_time(unsigned short timeout);

#endif /*sb_main_h*/
