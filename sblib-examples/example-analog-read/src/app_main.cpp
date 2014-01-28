/*
 *  A simple application that reads the analog channel AD0 and prints the
 *  read value to the serial port.
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
    analogBegin();
    pinMode(PIO0_11, INPUT_ANALOG);

    // Enable the serial port with 19200 baud, no parity, 1 stop bit
    serial.begin(19200);
    serial.println("Analog read example");
}

/*
 * The main processing loop.
 */
void loop()
{
    int value = analogRead(AD0);

    serial.println(value);

    delay(500);
}
