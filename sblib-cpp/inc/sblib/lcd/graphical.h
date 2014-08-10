/*
 *  graphical.h - base class for graphical LCD displays.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_lcd_graphical_h
#define sblib_lcd_graphical_h

#include <sblib/print.h>

class Font;


/**
 * Base class for graphical LCD displays.
 */
class LcdGraphical: public Print
{
public:
    /**
     * Clear the screen.
     */
    void clear();

    /**
     * Position the cursor. 0/0 is the top/left position
     * on the display.
     *
     * @param x - the X position, starting with 0
     * @param y - the Y position, starting with 0
     */
    virtual void pos(int x, int y) = 0;

    /**
     * Set the font for text output.
     *
     * @param font - the font to use.
     */
    void font(const Font& font);

protected:
    /**
     * Create a graphical LCD display.
     *
     * @param font - the font to use for text output, e.g. font_5x7
     * @param width - the width of the display in pixels
     * @param height - the height of the display in pixels
     */
    LcdGraphical(const Font& font, int width, int height);

    const Font* fnt;
    const int width, height;
};


inline void LcdGraphical::font(const Font& fnt)
{
    this->fnt = &fnt;
}

#endif /*sblib_lcd_graphical_h*/
