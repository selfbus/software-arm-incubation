/*
 *  weatherstaion.h - The application's main definitions.
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#ifndef WEATHERSTATION_H_
#define WEATHERSTATION_H_

enum
{
    COM_OBJ_BRIGHTNESS_EAST         =  0
,   COM_OBJ_THRESHOLD_1_EAST        =  1
,   COM_OBJ_THRESHOLD_2_EAST        =  2
,   COM_OBJ_THRESHOLD_1_EAST_LOWER  =  3
,   COM_OBJ_THRESHOLD_1_EAST_UPPER  =  4
,   COM_OBJ_BRIGHTNESS_EAST_BLOCK   =  5
,   COM_OBJ_FACADE_EAST_AUTO_1      =  6
,   COM_OBJ_FACADE_EAST_AUTO_2      =  7
,   COM_OBJ_FACADE_EAST_POS_SHUTTER =  6
,   COM_OBJ_FACADE_EAST_POS_SLATS   =  7

,   COM_OBJ_BRIGHTNESS_SOUTH        = 17
,   COM_OBJ_THRESHOLD_1_SOUTH       = 18
,   COM_OBJ_THRESHOLD_2_SOUTH       = 19
,   COM_OBJ_THRESHOLD_1_SOUTH_LOWER = 20
,   COM_OBJ_THRESHOLD_1_SOUTHT_UPPER= 21
,   COM_OBJ_BRIGHTNESS_SOUTH_BLOCK  = 22
,   COM_OBJ_FACADE_SOUTH_AUTO_1     = 23
,   COM_OBJ_FACADE_SOUTH_AUTO_2     = 24

,   COM_OBJ_BRIGHTNESS_WEST         = 34
,   COM_OBJ_THRESHOLD_1_WEST        = 35
,   COM_OBJ_THRESHOLD_2_WEST        = 36
,   COM_OBJ_THRESHOLD_1_WEST_LOWER  = 37
,   COM_OBJ_THRESHOLD_1_WEST_UPPER  = 38
,   COM_OBJ_BRIGHTNESS_WEST_BLOCK   = 39
,   COM_OBJ_FACADE_WEST_AUTO_1      = 40
,   COM_OBJ_FACADE_WEST_AUTO_2      = 41

,   COM_OBJ_STATUS = 58
};

#endif /* WEATHERSTATION_H_ */
