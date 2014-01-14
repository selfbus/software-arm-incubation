/*
 *  addr_tables.cpp - BCU communication address tables.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/addr_tables.h>

#include <sblib/eib/user_memory.h>


int indexOfAddr(int addr)
{
    byte* tab = addrTable();
    int num = *tab++;

    int addrHigh = addr >> 8;
    int addrLow = addr & 255;

    for (int i = 1; i < num; ++i, tab += 2)
    {
        if (tab[0] == addrHigh && tab[1] == addrLow)
            return i;
    }

    return -1;
}

int objectOfAddr(int addr)
{
    int addrIndex = indexOfAddr(addr);

    byte* tab = assocTable();
    int num = *tab++;

    for (int i = 0; i < num; ++i, tab += 2)
    {
        if (tab[0] == addrIndex)
            return tab[1];
    }

    return -1;
}

int addrForWriteObject(int objno)
{
    return 0;
}

byte* addrTable()
{
    return (byte*) &userEeprom.addrTabSize;
}

byte* assocTable()
{
#if BCU_TYPE == 0x10
    return ((byte*) &userEeprom) + userEeprom.assocTabPtr;
#else
#   error Unsupported BCU_TYPE
#endif
}
