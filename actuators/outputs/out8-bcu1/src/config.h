/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <sblib/ioports.h>

#define NO_OF_CHANNELS 8
#define BI_STABLE
#define NO_OF_HAND_PINS  8
// #define ZERO_DETECT

#define HAND_ACTUATION

#define VBUS_AD_CHANNEL AD7
#define VBUS_THRESHOLD 1.94 // TODO 1.94V @ ADC-Pin of the LPC11xx, 1.94V is just selected for fast testing, needs further investigation

#endif /* CONFIG_H_ */
