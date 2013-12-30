/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef com_objs_h
#define com_objs_h

/**
 * A structure for the com-object values.
 */
typedef struct ComObjectValues
{
    unsigned char primary[8];   // the primary input values
    unsigned char secondary[8]; // the secondary input values
    unsigned char lock[8];      // the lock values
} ComObjectValues;

// User ram address: value of com-object #0
#define UR_COM_OBJ_VALUE0              0x0053

// Eeprom address: telegram rate limit active (bit 2)
#define EE_TEL_RATE_LIMIT_ACTIVE       0x0010

// Eeprom address: telegram rate limit: telegrams per 17 sec
#define EE_TEL_RATE_LIMIT              0x00d3

// Eeprom address: input debounce time in 0.5 msec
#define EE_INPUT_DEBOUNCE_TIME         0x00d2

// Eeprom address: bit 0-6: bus power return start delay factor
#define EE_BUS_RETURN_DELAY_FACT       0x00d4

// Eeprom address: bit 4-7: bus power return start delay base
#define EE_BUS_RETURN_DELAY_BASE       0x00fe

// Eeprom address: bit 0-3: input 1 type (4 bit). See INPUT_TYPE_xx defines
#define EE_INPUT1_TYPE                 0x00ce


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
