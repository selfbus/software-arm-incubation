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

#endif /*sb_main_h*/
