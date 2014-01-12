/*
 *  bcu_2.h - Definitions for the BCU 2.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_bcu_2_h
#define sblib_bcu_2_h

//
//  Do not include this file directly, include <sblib/eib/bcu.h> instead!
//


/**
 * The size of the user RAM in bytes.
 */
#define USER_RAM_SIZE 256

/**
 * The size of the user EEPROM in bytes.
 */
#define USER_EEPROM_SIZE 1024

/**
 * Start address of the user RAM when ETS talks with us.
 */
#define USER_RAM_START 0

/**
 * Start address of the user EEPROM when ETS talks with us.
 */
#define USER_EEPROM_START 0x100


#endif /*sblib_bcu_2_h*/
