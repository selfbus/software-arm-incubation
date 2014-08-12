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
    bcu.begin(2, 1, 1); // ABB, dummy something device

    // Disable telegram processing by the lib
    if (userRam.status & BCU_STATUS_TL)
        userRam.status ^= BCU_STATUS_TL | BCU_STATUS_PARITY;

    serial.begin(19200);
    serial.println("Selfbus Bus Monitor");

    pinMode(PIO2_6, OUTPUT);	// Info LED
    pinMode(PIO3_3, OUTPUT);	// Run LED
}

/*
 * The main processing loop.
 */
void loop()
{
	digitalWrite(PIO3_3, 1);

	if (bus.telegramReceived())
    {
        for (int i = 0; i < bus.telegramLen; ++i)
        {
            if (i) serial.print(" ");
            serial.print(bus.telegram[i], HEX, 2);
        }
        serial.println();

        bus.discardReceivedTelegram();

        digitalWrite(PIO2_6, !digitalRead(PIO2_6));
    }

    // Sleep until the next 1 msec timer interrupt occurs (or shorter)
    __WFI();
}
