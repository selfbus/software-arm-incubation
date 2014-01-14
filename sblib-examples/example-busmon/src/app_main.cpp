/*
 *  app_main.cpp - The application's main.
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
    // FIXME remove when EEPROM writing is ported
    bcu.setOwnAddress(0x00fe); // 0.0.254

    // Disable telegram processing by the lib
    if (userRam.status & BCU_STATUS_TL)
        userRam.status ^= BCU_STATUS_TL | BCU_STATUS_PARITY;

    serial.begin(19200);
    serial.println("Selfbus Bus Monitor");
}

/*
 * The main processing loop.
 */
void loop()
{
    if (bus.telegramReceived())
    {
        int sz = telegramSize(bus.telegram);
        for (int i = 0; i < sz; ++i)
        {
            if (i) serial.print(" ");
            serial.print(bus.telegram[i], HEX, 2);
        }
        serial.println();

        bus.discardReceivedTelegram();
    }

    // Sleep until the next 1 msec timer interrupt occurs (or shorter)
    __WFI();
}
