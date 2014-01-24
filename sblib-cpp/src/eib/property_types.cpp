/*
 *  property_types.cpp - BCU 2 property types of EIB objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#if BCU_TYPE >= 20

#include <sblib/eib/property_types.h>

#include <sblib/eib/user_memory.h>


// The property sizes in bytes
const byte propertySizes[] =
{
    1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 8, 10, 3, 5, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};


int PropertyDef::size() const
{
    extern const byte propertySizes[];
    return propertySizes[control & PC_TYPE_MASK];
}

byte* PropertyDef::valuePointer() const
{
    if (control & PC_POINTER)
    {
        int offs = valAddr & PPT_OFFSET_MASK;

        switch (valAddr & PPT_MASK)
        {
        case PPT_USER_RAM:
            return userRamData + offs;
        case PPT_USER_EEPROM:
            return userEepromData + offs;
        default:
            fatalError(); // invalid property pointer type encountered
            break;
        }
    }

    return (byte*) &valAddr;
}

#endif /*BCU_TYPE > 20*/
