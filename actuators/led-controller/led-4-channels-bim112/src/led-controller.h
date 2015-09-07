/*
 *  led-controller.h - Application for a 4 channel LED controller
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef LED_CONTROLLER_H_
#define LED_CONTROLLER_H_

// IO definitions
#define RELAY_OUTPUT PIO1_0

enum
{   // RED/CHANNEL A
      COM_OBJ_RED_SWITCH                        =  0
    , COM_OBJ_CHANNEL_A_SWITCH                  =  0
    , COM_OBJ_CHANNEL_A_STAIRCASE               =  1
    , COM_OBJ_RED_CHANGE_REL                    =  2
    , COM_OBJ_CHANNEL_A_DIM_REL                 =  2
    , COM_OBJ_RED_CHANGE_ABS                    =  3
    , COM_OBJ_CHANNEL_A_DIM_ABS                 =  3
    , COM_OBJ_RED_STATE_ON_OFF                  =  4
    , COM_OBJ_CHANNEL_A_STATE_ON_OFF            =  4
    , COM_OBJ_RED_STATUS_VALUE                  =  5
    , COM_OBJ_CHANNEL_A_STATE_DIM_VALUE         =  5
    , COM_OBJ_CHANNEL_A_BLOCK_1                 =  6
    , COM_OBJ_CHANNEL_A_BLOCK_2                 =  7
    , COM_OBJ_CHANNEL_A_SCENE                   =  8
// dummy 9, 10
    , COM_OBJ_CHANNEL_A_AUTO_1                  = 11
    , COM_OBJ_CHANNEL_A_AUTO_2                  = 12
    , COM_OBJ_CHANNEL_A_AUTO_3                  = 13
    , COM_OBJ_CHANNEL_A_AUTO_4                  = 14

    // GREEN/CHANNEL B
    , COM_OBJ_GREEN_SWITCH                      = 15
    , COM_OBJ_CHANNEL_B_SWITCH                  = 15
    , COM_OBJ_CHANNEL_B_STAIRCASE               = 16
    , COM_OBJ_GREEN_CHANGE_REL                  = 17
    , COM_OBJ_CHANNEL_B_DIM_REL                 = 17
    , COM_OBJ_GREEN_CHANGE_ABS                  = 18
    , COM_OBJ_CHANNEL_B_DIM_ABS                 = 18
    , COM_OBJ_GREEN_STATE_ON_OFF                = 19
    , COM_OBJ_CHANNEL_B_STATE_ON_OFF            = 19
    , COM_OBJ_GREEN_STATUS_VALUE                = 20
    , COM_OBJ_CHANNEL_B_STATE_DIM_VALUE         = 20
    , COM_OBJ_CHANNEL_B_BLOCK_1                 = 21
    , COM_OBJ_CHANNEL_B_BLOCK_2                 = 22
    , COM_OBJ_CHANNEL_B_SCENE                   = 23
// dummy 24, 25
    , COM_OBJ_CHANNEL_B_AUTO_1                  = 26
    , COM_OBJ_CHANNEL_B_AUTO_2                  = 27
    , COM_OBJ_CHANNEL_B_AUTO_3                  = 28
    , COM_OBJ_CHANNEL_B_AUTO_4                  = 29

    // BLUE/CHANNEL C
    , COM_OBJ_BLUE_SWITCH                       = 30
    , COM_OBJ_CHANNEL_C_SWITCH                  = 30
    , COM_OBJ_CHANNEL_C_STAIRCASE               = 31
    , COM_OBJ_BLUE_CHANGE_REL                   = 32
    , COM_OBJ_CHANNEL_C_DIM_REL                 = 32
    , COM_OBJ_BLUE_CHANGE_ABS                   = 33
    , COM_OBJ_CHANNEL_C_DIM_ABS                 = 33
    , COM_OBJ_BLUE_STATE_ON_OFF                 = 34
    , COM_OBJ_CHANNEL_C_STATE_ON_OFF            = 34
    , COM_OBJ_BLUE_STATUS_VALUE                 = 35
    , COM_OBJ_CHANNEL_C_STATE_DIM_VALUE         = 35
    , COM_OBJ_CHANNEL_C_BLOCK_1                 = 36
    , COM_OBJ_CHANNEL_C_BLOCK_2                 = 37
    , COM_OBJ_CHANNEL_C_SCENE                   = 38
// dummy 39, 40
    , COM_OBJ_CHANNEL_C_AUTO_1                  = 41
    , COM_OBJ_CHANNEL_C_AUTO_2                  = 42
    , COM_OBJ_CHANNEL_C_AUTO_3                  = 43
    , COM_OBJ_CHANNEL_C_AUTO_4                  = 44

    // WHITE/CHANNEL D
    , COM_OBJ_WHITE_SWITCH                      = 45
    , COM_OBJ_CHANNEL_D_SWITCH                  = 45
    , COM_OBJ_CHANNEL_D_STAIRCASE               = 46
    , COM_OBJ_WHITE_CHANGE_REL                  = 47
    , COM_OBJ_CHANNEL_D_DIM_REL                 = 47
    , COM_OBJ_WHITE_CHANGE_ABS                  = 48
    , COM_OBJ_CHANNEL_D_DIM_ABS                 = 48
    , COM_OBJ_WHITE_STATE_ON_OFF                = 49
    , COM_OBJ_CHANNEL_D_STATE_ON_OFF            = 49
    , COM_OBJ_WHITE_STATUS_VALUE                = 50
    , COM_OBJ_CHANNEL_D_STATE_DIM_VALUE         = 50
    , COM_OBJ_CHANNEL_D_BLOCK_1                 = 51
    , COM_OBJ_CHANNEL_D_BLOCK_2                 = 52
    , COM_OBJ_CHANNEL_D_SCENE                   = 53
// dummy 54, 55
    , COM_OBJ_CHANNEL_D_AUTO_1                  = 56
    , COM_OBJ_CHANNEL_D_AUTO_2                  = 57
    , COM_OBJ_CHANNEL_D_AUTO_3                  = 58
    , COM_OBJ_CHANNEL_D_AUTO_4                  = 59

    , COM_OBJ_CENTRAL_SWITCH                    = 60
    , COM_OBJ_CENTRAL_DIM_ABS                   = 61
    , COM_OBJ_CENTRAL_ERROR                     = 62

    , COM_OBJ_RGB_SWITCH                        = 63
    , COM_OBJ_RGBW_SWITCH                       = 63
    , COM_OBJ_RGB_COLOR                         = 64
    , COM_OBJ_RGBW_COLOR                        = 64
    , COM_OBJ_HSV_COLOR                         = 65

    , COM_OBJ_H_ABS_VALUE                       = 66
    , COM_OBJ_S_ABS_VALUE                       = 67
    , COM_OBJ_V_ABS_VALUE                       = 68
    , COM_OBJ_H_REL_VALUE                       = 69
    , COM_OBJ_S_REL_VALUE                       = 70
    , COM_OBJ_V_REL_VALUE                       = 71

    , COM_OBJ_RGB_STATE_ON_OFF                  = 72
    , COM_OBJ_RGBW_STATE_ON_OFF                 = 72
    , COM_OBJ_RGB_STATUS_VALUE                  = 73
    , COM_OBJ_RGBW_STATUS_VALUE                 = 73
    , COM_OBJ_HSV_STATUS_VALUE                  = 74

    , COM_OBJ_H_ABS_STATUS_VALUE                = 75
    , COM_OBJ_S_ABS_STATUS_VALUE                = 76
    , COM_OBJ_V_ABS_STATUS_VALUE                = 77

    , COM_OBJ_RGB_SCENE                         = 78
    , COM_OBJ_RGBW_SCENE                        = 78
    , COM_OBJ_RGB_BIT_SCENE_1                   = 79
    , COM_OBJ_RGBW_BIT_SCENE_1                  = 79
    , COM_OBJ_RGB_BIT_SCENE_2                   = 80
    , COM_OBJ_RGBW_BIT_SCENE_2                  = 80

    , COM_OBJ_RGB_BLOCK                         = 81
    , COM_OBJ_RGBW_BLOCK                        = 81
    , COM_OBJ_RGB_TEACH_IN_WB                   = 82
    , COM_OBJ_RGBW_TEACH_IN_WB                  = 82

    , COM_OBJ_RGB_START_SEQUENCE_1              = 83
    , COM_OBJ_RGBW_START_SEQUENCE_1             = 83
    , COM_OBJ_RGB_START_SEQUENCE_2              = 84
    , COM_OBJ_RGBW_START_SEQUENCE_2             = 84
    , COM_OBJ_RGB_START_SEQUENCE_3              = 85
    , COM_OBJ_RGBW_START_SEQUENCE_3             = 85
    , COM_OBJ_RGB_START_SEQUENCE_4              = 86
    , COM_OBJ_RGBW_START_SEQUENCE_4             = 86
    , COM_OBJ_RGB_START_SEQUENCE_5              = 87
    , COM_OBJ_RGBW_START_SEQUENCE_5             = 87

    , COM_OBJ_CENTRAL_CURRENT_ALARM             = 88
    , COM_OBJ_RGB_CURRENT_ALARM                 = 88
    , COM_OBJ_RGBW_CURRENT_ALARM                = 88
    , COM_OBJ_CENTRAL_OVERTEMP_ALARM            = 89
    , COM_OBJ_RGB_OVERTEMP_ALARM                = 89
    , COM_OBJ_RGBW_OVERTEMP_ALARM               = 89

    , COM_OBJ_RELAI_SWITCH                      = 90
// dummy 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102
};

#endif /* LED_CONTROLLER_H_ */
