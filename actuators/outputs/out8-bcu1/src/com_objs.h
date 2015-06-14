/*
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
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
struct ObjectValues
{
    unsigned char outputs[8];            // the output values
    unsigned char special_functiions[4]; // the special function values
    unsigned char feedback[8];           // the feedback values
};

// User ram address: value of com-object #0
#define UR_COM_OBJ_VALUE0              0x0020

/*
 * Com objects
 */

// Primary com-object for output 1. The com-objects 1-7 are for output 2-8
#define COMOBJ_INPUT1          0

// Com-object for special function 1. The com-objects 9-11 are for special function 2-4
#define COMOBJ_SPECIAL1        8

// Feeback com-object for ouput 1. The com-objects 13-19 are for feedback for output 2-8
#define COMOBJ_FEEDBACK1      12

#endif /*com_objs_h*/
