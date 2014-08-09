/*
 *  An example application for a graphical LCD.
 *  We use the EA DOGS 102, a 102x64 monochrome graphical LCD.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/lcd/graphical_eadogs.h>
#include <sblib/lcd/font_5x7.h>
#include <sblib/core.h>
#include <sblib/ioports.h>


/*
 * We use SPI port 0 in this example.
 *
 * Connect the ARM in this way to the EA-DOGS:
 *
 * PIO0_2:  SSEL0 -> Display CS0 "chip select"
 * PIO0_9:  MOSI0 -> Display SDA "data in"
 * PIO2_11: SCK0  -> Display SCK "clock"
 * PIO0_8:        -> Display CD  "command/data"
 */


LcdGraphicalEADOGS display(SPI_PORT_0, PIO0_9, PIO2_11, PIO0_8, PIO0_2, font_5x7);

int blinkPin = PIO0_7;


/*
 * Initialize the application.
 */
void setup()
{
    display.begin();
    pinMode(blinkPin, OUTPUT);
}

/*
 * The main processing loop.
 */
void loop()
{
    digitalWrite(blinkPin, !digitalRead(blinkPin));
    delay(500);
}
