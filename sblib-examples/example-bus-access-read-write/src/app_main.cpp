/*
 *  app_main.cpp - The application's main.
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
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

    serial.begin(115200);
    serial.println("Selfbus Bus Monitor");

    pinMode(PIO2_6, OUTPUT);	// Info LED
    pinMode(PIO3_3, OUTPUT);	// Run LED
}

/*
 * Convert a character from the serial line into a number
 */
static unsigned char _byteToNumber(unsigned char data)
{
	unsigned char result;
	result = data - 48;
	if (result > 9)
	{
		result -= 7;
		if (result > 15)
			result -= 32;
	}
	return result;
}

/*
 * The main processing loop.
 */
void loop()
{
	static int receiveCount = -1;
	static int telLength = 0;
	static int  byteCount = 0;
	static unsigned char telegram[32];
	static unsigned char data;

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

	// handle the incoming data form the serial line
	// the format should be:
	// CC aa bb cc dd ee
	// where CC is the number of bytes of the the telegram to send
	//       aa,bb,cc,dd,ee   the telegram data (without the checksum)
	if (serial.available())
	{
		int byte = serial.read();
		byteCount++;
		data = (data << 4) + _byteToNumber(byte);
		if (byteCount == 2)
		{   // a new data byte has been received
			byteCount = 0;
			serial.print(data, HEX, 2);
			serial.print(" ");
			if (receiveCount < 0)
			{   // the first byte is the number of bytes
				receiveCount = data;
			}
			else
			{
				telegram[telLength++] = data;
				if (telLength == receiveCount)
				{
					bus.sendTelegram(telegram, telLength);
					telLength = 0;
					receiveCount = -1;
					serial.println();
				}
			}
		}
	}
    // Sleep until the next 1 msec timer interrupt occurs (or shorter)
    __WFI();
}
