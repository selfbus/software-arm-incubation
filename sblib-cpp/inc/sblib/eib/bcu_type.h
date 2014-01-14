/*
 *  bcu_type.h - BCU type definitions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_bcu_type_h
#define sblib_bcu_type_h

// Fallback to BCU1 if BCU_TYPE is not defined
#ifndef BCU_TYPE
# define BCU_TYPE 0x10
#endif


#if BCU_TYPE == 0x10

    /** Start address of the user RAM when ETS talks with us. */
#   define USER_RAM_START 0x50

    /** The size of the user RAM in bytes. */
#   define USER_RAM_SIZE 176

    /** Start address of the user EEPROM when ETS talks with us. */
#   define USER_EEPROM_START 0x100

    /** The size of the user EEPROM in bytes. */
#   define USER_EEPROM_SIZE 256

#elif BCU_TYPE == 0x20

    /** Start address of the user RAM when ETS talks with us. */
#   define USER_RAM_START 0x50

    /** The size of the user RAM in bytes. */
#   define USER_RAM_SIZE 176

    /** Start address of the user EEPROM when ETS talks with us. */
#   define USER_EEPROM_START 0x100

    /** The size of the user EEPROM in bytes. */
#   define USER_EEPROM_SIZE 1023


#else /* BCU_TYPE contains an unknown value. */

#   error "Unsupported BCU type"

#endif


/** End address of the user RAM +1, when ETS talks with us. */
#define USER_RAM_END (USER_RAM_START + USER_RAM_SIZE)

/** End address of the user EEPROM +1, when ETS talks with us. */
#define USER_EEPROM_END (USER_EEPROM_START + USER_EEPROM_SIZE)


#endif /*sblib_bcu_type_h*/
