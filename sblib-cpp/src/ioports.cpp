/*
 *  ioports.h - Definition of the I/O ports and port pins.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include <sblib/ioports.h>


const int portMask[12] =
{
    1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048
};


short getPinFunctionNumber(int pin, short func)
{
    pin >>= PF0_SHIFT;

    for (int funcNumber = 0; funcNumber < 4; ++funcNumber)
    {
        if ((pin & PFF_MASK) == func)
            return funcNumber;

        pin >>= PFF_SHIFT_OFFSET;
    }

    return -1;
}
