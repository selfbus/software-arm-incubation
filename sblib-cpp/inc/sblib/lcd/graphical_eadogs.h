/*
 *  graphical_eadogs.h - class for EA-DOGS graphical LCD displays.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_lcd_graphical_eadogs_h
#define sblib_lcd_graphical_eadogs_h

#include <sblib/lcd/graphical.h>
#include <sblib/ioports.h>
#include <sblib/spi.h>


/**
 * Class for EA-DOGS graphical LCD displays. These displays have a UC1701 display
 * controller.
 *
 * The fonts are stored in separate files. The default font can be used with
 * #include <sblib/lcd/font_5x7.h> and is named font_5x7.
 *
 * WARNING
 *
 * This code is only half done !
 *
 */
class LcdGraphicalEADOGS: public LcdGraphical
{
public:
    /**
     * Create a graphical LCD display for a EA-DOGS display. The pins pinData must be capable
     * of SPI MOSI for the selected SPI port, the pin pinClock must be capable of SPI CLK for
     * the selected SPI port.
     *
     * @param spiPort - the SPI port to use, e.g. SPI_PORT_0
     * @param pinData - the digital pin for SPI data transfer, e.g. PIO0_9
     * @param pinClock - the digital pin for SPI clock, e.g. PIO2_11
     * @param pinCD - the digital pin for switching between command and data, e.g. PIO0_8
     * @param pinCS - the digital pin for chip select, e.g. PIO0_2. May be 0 for no chip select.
     * @param font - the font to use for text output, e.g. font_5x7
     */
    LcdGraphicalEADOGS(int spiPort, int pinData, int pinClock, int pinCD, int pinCS, const Font& font);

    /**
     * Position the cursor. 0/0 is the top/left position
     * on the display.
     *
     * @param x - the X position, starting with 0
     * @param y - the Y position, starting with 0
     */
    virtual void pos(int x, int y);

    /**
     * Begin using the display.
     */
    void begin();

    /**
     * End using the display.
     */
    void end();

protected:
    SPI spi;
    const int pinCD, pinCS;
};


#endif /*sblib_lcd_graphical_eadogs_h*/
