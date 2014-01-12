/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sb_main_h
#define sb_main_h

#include "sb_memory.h"

// Define trick to ensure that it is not possible to link a program that is
// compiled with SB_BCU1 with the SB_BCU2 library and vice versa. This trick
// renames the function sb_init() to either sb_init_bcu1() or sb_init_bcu2().
#ifdef SB_BCU2
#   define sb_init  sb_init_bcu2
#else
#   define sb_init  sb_init_bcu1
#endif


/**
 * Initialize the library. Call this function once when the program starts.
 *
 * If you get an error when linking your program, saying that sb_init_bcu1 or
 * sb_init_bcu2 is missing, then the define constants SB_BCU1 / SB_BCU2 of
 * library and application do not match.
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

#endif /*sb_main_h*/
