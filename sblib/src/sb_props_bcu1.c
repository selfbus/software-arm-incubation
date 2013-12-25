/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sb_props.h"
#include "sb_memory.h"

#ifdef SB_BCU1

void sb_props_read_tel(unsigned char objectIdx, unsigned char propertyId,
                       unsigned char count, unsigned short start)
{
    // Dummy implementation: unused for BCU1
}

#endif //SB_BCU1
