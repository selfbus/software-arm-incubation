/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef params_h
#define params_h

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


/** Command category: input pin changed. */
#define CMD_CAT_PINCHANGE 0x100

/** Command category: lock a channel. */
#define CMD_CAT_LOCK      0x200

/** Command category: unlock a channel. */
#define CMD_CAT_UNLOCK    0x300

/** Command category: bus power on. */
#define CMD_CAT_POWERON   0x400

/** Command: do nothing. */
#define CMD_NONE          0

/** Command: on. */
#define CMD_ON            1

/** Command: off. */
#define CMD_OFF           2

/** Command: toggle. */
#define CMD_TOGGLE        3

/** Command: like rising edge. */
#define CMD_LIKE_RISING   1

/** Command: like falling edge. */
#define CMD_LIKE_FALLING  2

/** Command: current value. */
#define CMD_CURRENT       3



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

#endif /*params_h*/
