/*
 *  graphical.h - base class for graphical LCD displays.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/lcd/graphical.h>
#include <sblib/lcd/font.h>


LcdGraphical::LcdGraphical(const Font& fnt, int width, int height)
:fnt(&fnt)
,width(width)
,height(height)
{
}

int LcdGraphical::write(byte ch)
{
}
