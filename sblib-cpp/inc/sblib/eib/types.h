/*
 *  types.h - EIB data types
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_eib_types_h
#define sblib_eib_types_h

#include <sblib/types.h>


/**
 * The type of a communication object.
 */
enum ComType
{
    /** Communication object type: 1 bit */
    BIT_1 = 0,

    /** Communication object type: 2 bit */
    BIT_2 = 1,

    /** Communication object type: 3 bit */
    BIT_3 = 2,

    /** Communication object type: 4 bit */
    BIT_4 = 3,

    /** Communication object type: 5 bit */
    BIT_5 = 4,

    /** Communication object type: 6 bit */
    BIT_6 = 5,

    /** Communication object type: 7 bit */
    BIT_7 = 6,

    /** Communication object type: 1 byte */
    BYTE_1 = 7,

    /** Communication object type: 2 bytes */
    BYTE_2 = 8,

    /** Communication object type: 3 bytes */
    BYTE_3 = 9,

    /** Communication object type: float */
    FLOAT = 10,

    /** Communication object type: 6 bytes */
    DATA_6 = 11,

    /** Communication object type: double */
    DOUBLE = 12,

    /** Communication object type: 10 bytes */
    DATA_10 = 13,

    /** Communication object type: 14 bytes */
    MAXDATA = 14,

    /** Communication object type: variable length 1-14 bytes */
    VARDATA = 15
};

#endif /*sblib_eib_types_h*/
