/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sb_props.h"
#include "sb_memory.h"

#ifdef SB_BCU1

unsigned char sb_props_read_tel(unsigned char objectIdx, unsigned char propertyId,
                                unsigned char count, unsigned short start)
{
    return 0;
}

unsigned char sb_props_write_tel(unsigned char objectIdx, unsigned char propertyId,
                                 unsigned char count, unsigned short start)
{
    return 0;
}

#endif //SB_BCU1
