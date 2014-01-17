/*
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef com_objs_h
#define com_objs_h

#include <sblib/types.h>

/**
 * The secondary channel values can either be 1 or 2 bytes long. For 1 byte
 * values use b[0], for 2 byte values use s.
 */
union SecondaryValue
{
    byte b[2];         //!< byte values, use b[0] (b[1] is unused)
    unsigned short s;  //!< 2 byte values for
};

/**
 * A structure for the com-object values.
 */
struct ObjectValues
{
    byte primary[8];             //!< the primary values
    SecondaryValue secondary[8]; //!< the secondary values, either 1 or 2 bytes each
    byte lock[8];                //!< the lock values
    byte lastLock[8];            //!< the previous lock values
};

// User ram address: value of com-object #0
#define UR_COM_OBJ_VALUE0              0x0053


// Input type: unused / no function
#define INPUT_TYPE_NONE          0

// Input type: switching
#define INPUT_TYPE_SWITCH        8

// Input type: dimming
#define INPUT_TYPE_DIM           4

// Input type: blind / jalousie
#define INPUT_TYPE_JALO          12

// Input type: encoder / value sender
#define INPUT_TYPE_ENCODER       2

// Input type: impulse counter (input 1+2 only)
#define INPUT_TYPE_COUNT_IMPULSE 10

// Input type: switch counter (input 1+2 only)
#define INPUT_TYPE_COUNT_SWITCH  6


/*
 * Com objects
 */

// Primary com-object for input 1. The com-objects 1-7 are for input 2-8
#define COMOBJ_PRIMARY1          0

// Secondary com-object for input 1. The com-objects 9-15 are for input 2-8
#define COMOBJ_SECONDARY1        8

// Lock com-object for input 1. The com-objects 17-23 are for input 2-8
#define COMOBJ_LOCK1            16

#endif /*com_objs_h*/
