/*
 *  font.h - a font for graphical LCD displays.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/lcd/font.h>

Font::Font(int firstChar, int numChars, int charWidth, const char* data)
:firstChar(firstChar)
,numChars(numChars)
,charWidth(charWidth)
,data(data)
{
}
