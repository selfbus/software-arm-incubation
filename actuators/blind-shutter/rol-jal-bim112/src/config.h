/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <sblib/types.h>
#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>

typedef struct
{
    unsigned int noOfChannels;  //!> how many channels are supported with this hardware
    unsigned short baseAddress; //!> Base address of the config parameters
    byte hardwareVersion[6];    //!> The hardware identification number
} HardwareVersion;

extern const HardwareVersion * currentVersion;


//#define HAND_ACTUATION

//#define USE_DEV_LEDS
//#define MEM_TEST


/*
 *  hand actuation pin configuration
 */
#ifdef HAND_ACTUATION
#   define NO_OF_HAND_PINS 8
#   define READBACK_PIN PIN_LT9
#   define BLINK_TIME 500
    // LedTaster_4TE_3.54 : links oben = Kanal 1 nach unten aufsteigende Kanalnummer ; rechts oben = Kanal 5 ; nach unten aufsteigende Kanalnummer
    const unsigned int handPins[NO_OF_HAND_PINS] = { PIN_LT1, PIN_LT2, PIN_LT3, PIN_LT4, PIN_LT5, PIN_LT6, PIN_LT7, PIN_LT8 };
#endif

#endif /* CONFIG_H_ */
