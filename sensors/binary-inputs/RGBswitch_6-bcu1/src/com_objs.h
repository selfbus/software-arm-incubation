/*
 *  Copyright (c) 2018 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef com_objs_h
#define com_objs_h

enum ChannelType
{
    /** Channel type: unused / no function */
    CHANNEL_TYPE_NONE = 0,

    /** Channel type: switching */
    CHANNEL_TYPE_SWITCH = 1,

    /** Channel type: dimming */
    CHANNEL_TYPE_DIM = 2,

    /** Channel type: blind / jalousie */
    CHANNEL_TYPE_JALO = 3,

    /** Channel type: dimming encoder / value sender */
    CHANNEL_TYPE_DIM_ENCODER = 4,

    /** Channel type: light scene sender without storing */
    CHANNEL_TYPE_SCENE_NOSTORE = 5,

    /** Channel type: light scene sender with storing */
    CHANNEL_TYPE_SCENE_STORE = 6,

    /** Channel type: temperature encoder / value sender */
    CHANNEL_TYPE_TEMP_ENCODER = 7,

    /** Channel type: luminosity encoder / value sender */
    CHANNEL_TYPE_LUM_ENCODER = 8,

    /** Channel type: impulse counter on input 1/2 with synchronization on inputs 3/4 */
    CHANNEL_TYPE_COUNT_IMPULSE_1 = 9,

    /** Channel type: switch counter on input 1/2 */
    CHANNEL_TYPE_COUNT_IMPULSE_2 = 10
};


enum DimmerType
{
	DIMMER_TYPE_ONE_HAND                = 0,
	DIMMER_TYPE_TWO_HAND_LIGHTER_ON     = 1,
	DIMMER_TYPE_TWO_HAND_DARKER_OFF     = 2,
	DIMMER_TYPE_TWO_HAND_LIGHTER_TOGGLE = 3,
	DIMMER_TYPE_TWO_HAND_DARKER_TOGGLE  = 4
};

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
