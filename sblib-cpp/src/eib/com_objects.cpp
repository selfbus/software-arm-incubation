/*
 *  com_objects.cpp - EIB Communication objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/com_objects.h>


void ComObjects::set(short objno, int value)
{
    ComType t = type(objno);
}

void ComObjects::set(short objno, byte* value)
{
    ComType t = type(objno);
}

int ComObjects::asInt(short objno) const
{
    return 0;
}

byte* ComObjects::asArray(short objno) const
{
    return 0;
}

ComType ComObjects::type(short objno) const
{
    return BIT_1;
}

short ComObjects::size(short objno) const
{
    return 0;
}

short ComObjects::count() const
{
    return 0;
}
