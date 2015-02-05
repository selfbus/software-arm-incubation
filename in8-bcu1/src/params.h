/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef params_h
#define params_h

#include <sblib/timeout.h>
#include <sblib/debounce.h>

/**
 * Number of input channels. Default: 8.
 * Can be set at compile time.
 */
#ifndef NUM_CHANNELS
#  define NUM_CHANNELS 8
#endif

typedef union
{
    struct
    {
        unsigned char started;
    } dim;
    struct
    {
        bool        first;
        signed char step;
        signed int  value;
    } dimenc;
} ChannelData;

extern const int inputPins[];
extern Debouncer inputDebouncer[NUM_CHANNELS];
extern ChannelData channelData[NUM_CHANNELS];

/**
 * Calculate the time in ms based on a time base and a factor
 */
unsigned int calculateTime(int base, int factor);

/**
 * The channel parameters. 4 bytes per channel.
 * In userEeprom at address 0x01d5
 */
extern const byte* channelParams;

/**
 * Channel timing parameters. 1 byte per channel.
 * In userEeprom at address 0x01f6
 */
extern const byte* channelTimingParams;

/**
 * pointer to the configuration of the currently evaluated channel
 */
extern const byte* params;

extern Timeout timeout[NUM_CHANNELS * 2];
extern unsigned int delayTime[NUM_CHANNELS * 2];
//extern int lastLock[NUM_CHANNELS];


// Eeprom address: telegram rate limit active (bit 2)
#define EE_TEL_RATE_LIMIT_ACTIVE       0x0110

// Eeprom address: input debounce time in 0.5 msec
#define EE_INPUT_DEBOUNCE_TIME         0x01d2

// Eeprom address: telegram rate limit: telegrams per 17 sec
#define EE_TEL_RATE_LIMIT              0x01d3

// Eeprom address: bit 0-6: start delay factor on bus power return
#define EE_BUS_RETURN_DELAY_FACT       0x01d4

// Eeprom address: bit 4-7: start delay base on bus power return
#define EE_BUS_RETURN_DELAY_BASE       0x01fe

// Eeprom address: bit 0-3: input 1 type (4 bit). See INPUT_TYPE_xx defines
#define EE_INPUT1_TYPE                 0x01ce

// Eeprom address: start of channel parameters
#define EE_CHANNEL_PARAMS_BASE         0x01d5

// Eeprom address: start of channel timing parameters
#define EE_CHANNEL_TIMING_PARAMS_BASE  0x01f6

#define EE_INPUT_SWITCH_LOCK_NO_REACTION 0
#define EE_INPUT_SWITCH_LOCK_SET_ON      1
#define EE_INPUT_SWITCH_LOCK_SET_OFF     2
#define EE_INPUT_SWITCH_LOCK_TOGGLE      3
#define EE_INPUT_SWITCH_LOCK_SET_CURRENT 3

#define EE_INPUT_DIM_LOCK_NO_REACTION    0
#define EE_INPUT_DIM_LOCK_ON_SET_ON      1
#define EE_INPUT_DIM_LOCK_ON_SET_OFF     2
#define EE_INPUT_DIM_LOCK_ON_TOGGLE      3
#define EE_INPUT_DIM_LOCK_OFF_SET_OFF    1

#define EE_JALO_BUS_RETURN_NO_ACTION     0
#define EE_JALO_BUS_RETURN_DOWN			 1
#define EE_JALO_BUS_RETURN_UP			 2
#define EE_JALO_LOCK_NO_REACTION 		 0
#define EE_JALO_LOCK_SET_DOWN      		 1
#define EE_JALO_LOCK_SET_UP     		 2
#define EE_JALO_LOCK_TOGGLE      		 3

#define EE_DIMENC_TIME_LONG              5000
#endif /*params_h*/
