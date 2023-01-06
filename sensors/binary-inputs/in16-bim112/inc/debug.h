/*
 * debug.h
 *
 *  Created on: 08.08.2015
 *      Author: Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef SERIAL_DEBUG

#include <sblib/serial.h>
#include <sblib/ioports.h>

inline void debug_eeprom(const char *name, unsigned int channelConfig,
        unsigned int len)
{
    serial.println(name);
    for (int i = 0; i < len; ++i)
    {
        if (i)
            serial.print(" ");
        serial.print(userEeprom.getUInt8(channelConfig + i), HEX, 2);
    }
    serial.println();
}

inline void debug_init(void)
{
    serial.setRxPin(PIO3_1);
    serial.setTxPin(PIO3_0);
    serial.begin(115200);
    serial.println("Online\n");
}
#else
#define debug_init()
#define debug_eeprom(x,y,z)
#endif

#endif /* DEBUG_H_ */
