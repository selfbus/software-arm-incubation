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

typedef struct
{
    unsigned int noOfChannels;  //!> how many channels are supported with this hardware
    unsigned short baseAddress; //!> Base address of the config parameters
    byte hardwareVersion[6];    //!> The hardware identification number
} HardwareVersion;

extern const HardwareVersion * currentVersion;

#endif /* CONFIG_H_ */
