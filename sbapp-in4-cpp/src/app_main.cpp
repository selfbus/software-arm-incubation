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
 * Process the changed communication object.
 */
void processObject(int objno)
{

}

/*
 * Initialize the application.
 */
void setup()
{
//    // FIXME remove when eeprom writing is ported
//    bcu.setOwnAddress(0x117d); // 1.1.125


//    bcu.appData(0, 2, 0x9009, 0x01); // We are a ABB TSU/4.2 device, version 0.1
    bcu.appData(0, 4, 0x7054, 2);    // We are a "Jung 2118" device, version 0.2
}

/*
 * The main processing loop.
 */
void loop()
{
    int objno;

    // Handle updated communication objects
    while ((objno = nextUpdatedObject()) >= 0)
    {
        processObject(objno);
    }

    // Sleep until the next 1 msec timer interrupt occurs (or shorter)
    if (bus.idle())
        __WFI();
}
