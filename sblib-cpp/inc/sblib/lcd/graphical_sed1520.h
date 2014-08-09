/*
 *  graphical_sed1520.h - class for graphical LCD display with a SED1520
 *                        compatible controller.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_lcd_graphical_sed1520_h
#define sblib_lcd_graphical_sed1520_h

#include <sblib/lcd/graphical.h>
#include <sblib/ioports.h>


/**
 * Class for graphical LCD display with a SED1520 compatible controller.
 *
 * The fonts are stored in separate files. The default font can be used with
 * #include <sblib/lcd/font_5x7.h> and is named font_5x7.
 *
 * WARNING
 *
 * This code is only half done !
 *
 */
class LcdGraphicalSED1520: public LcdGraphical
{
public:
    /**
     * Create a graphical LCD display for a SED1520 compatible controller.
     *
     * @param dataPort - the IO port for the data lines: PIO0, PIO1, PIO2, PIO3
     * @param pinA0 - the digital pin for instruction/data select
     * @param pinCS1 - the digital pin for chip select 1
     * @param pinCS2 - the digital pin for chip select 2
     * @param pinRD - the digital pin for read active
     * @param pinWR - the digital pin for write active
     * @param font - the font to use for text output, e.g. font_5x7
     */
    LcdGraphicalSED1520(int dataPort, int pinA0, int pinCS1, int pinCS2, int pinRD,
        int pinWR, const Font& font);

    /**
     * Position the cursor. 0/0 is the top/left position
     * on the display.
     *
     * @param x - the X position, starting with 0
     * @param y - the Y position, starting with 0
     */
    virtual void pos(int x, int y);

protected:
    /**
     * Wait until the display is idle.
     */
    void idle();

protected:
    const int dataPort, pinA0, pinCS1, pinCS2, pinRD, pinWR;
};


#endif /*sblib_lcd_graphical_sed1520_h*/
