/*
 *  A simple example for SPI.
 *
 *  This example configures SPI for output and sends a byte every second.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/core.h>
#include <sblib/ioports.h>
#include <sblib/spi.h>


/*
 * We use SPI port 0 in this example.
 *
 * Pinout:
 *
 * PIO0_2:  SSEL0
 * PIO0_9:  MOSI0
 * PIO2_11: SCK0
 */


SPI spi(SPI_PORT_0);

int blinkPin = PIO0_7;


/*
 * Initialize the application.
 */
void setup()
{
    pinMode(blinkPin,  OUTPUT);

    pinMode(PIO0_2,  OUTPUT | SPI_SSEL);
    pinMode(PIO0_9,  OUTPUT | SPI_MOSI);
    pinMode(PIO2_11, OUTPUT | SPI_CLOCK);

    spi.setClockDivider(128);
    spi.begin();
}

/*
 * The main processing loop.
 */
void loop()
{
    static int val = 0;

    val++;
    val &= 255;

    digitalWrite(blinkPin, true);
    spi.transfer(val);
    delay(200);

    digitalWrite(blinkPin, false);
    delay(800);
}
