/*
 *  main.cpp - The application's main.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib.h>


/*
 * Initialize the application.
 */
void setup()
{
    // FIXME remove when eeprom writing is ported
    bcu.setOwnAddress(0x117d); // 1.1.125

    bcu.appData(0, 2, 0x9009, 0x01);  // we are a ABB TSU/4.2 version 0.1
}

/*
 * The main processing loop.
 */
void loop()
{
    int idx;

    // Handle updated communication objects
    while ((idx = nextUpdatedObject()) >= 0)
    {

    }

    // Sleep until the next 1 msec timer interrupt occurs (or shorter)
    if (bus.idle())
        __WFI();
}
