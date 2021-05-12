/*
 *  config.h - The application's config file.
 *
 *  Copyright (c) 2021 Dartyhson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>

#define NO_OF_CHANNELS 4
#define NO_OF_OUTPUTS  (NO_OF_CHANNELS * 2)

#define HAND_ACTUATION

#ifdef HAND_ACTUATION
#   define NO_OF_HAND_PINS NO_OF_OUTPUTS
#   define READBACK_PIN PIN_LT9
#   define BLINK_TIME 500
    // LedTaster_4TE_3.54 : links oben = Kanal 1 nach unten aufsteigende Kanalnummer ; rechts oben = Kanal 5 ; nach unten aufsteigende Kanalnummer
    const unsigned int handPins[NO_OF_HAND_PINS] = { PIN_LT1, PIN_LT2, PIN_LT3, PIN_LT4, PIN_LT5, PIN_LT6, PIN_LT7, PIN_LT8 };
#endif


#endif /* CONFIG_H_ */
