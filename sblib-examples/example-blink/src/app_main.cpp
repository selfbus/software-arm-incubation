/*
 *  A simple application that blinks the LED of the LPCxpresso board (on pin PIO0.7)
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/core.h>


/*
 * Initialize the application.
 */
void setup()
{
    pinMode(PIO0_7, OUTPUT);
}

/*
 * The main processing loop.
 */
void loop()
{
    digitalWrite(PIO0_7, !digitalRead(PIO0_7));
    delay(500);
}
