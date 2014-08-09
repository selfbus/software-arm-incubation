/*
 *  font.h - a font for graphical LCD displays.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_lcd_font_h
#define sblib_lcd_font_h

/**
 * A font for graphical LCD displays.
 *
 * @see sblib/lcd/font_5x7.h for a 5x7 font
 */
class Font
{
public:
    /**
     * Create a font object.
     *
     * @param firstChar - the first character in the font
     * @param numChars - the number of characters in the font
     * @param charWidth - the width of a character in pixel
     * @param data - the font data bytes, an array of numChars*charWidth size
     */
    Font(int firstChar, int numChars, int charWidth, const char* data);

    /**
     * The first character in the font.
     */
    const int firstChar;

    /**
     * The number of characters that the font has, starting with firstChar.
     */
    const int numChars;

    /**
     * The width of one character in pixel.
     */
    const int charWidth;

    /**
     * The raw font data bytes, an array of numChars*charWidth size.
     */
    const char* data;
};

#endif /*sblib_lcd_font_h*/
