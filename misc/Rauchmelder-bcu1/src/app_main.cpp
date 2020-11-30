/*
 *    _____ ________    __________  __  _______    ____  __  ___
 *   / ___// ____/ /   / ____/ __ )/ / / / ___/   / __ \/  |/  /
 *   \__ \/ __/ / /   / /_  / __  / / / /\__ \   / /_/ / /|_/ /
 *  ___/ / /___/ /___/ __/ / /_/ / /_/ /___/ /  / _, _/ /  / /
 * /____/_____/_____/_/   /_____/\____//____/  /_/ |_/_/  /_/
 *
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *  Copyright (c) 2017 Oliver Stefan <o.stefan252@googlemail.com>
 *  Copyright (c) 2020 Stefan Haller
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib.h>
#include <sblib/eib/sblib_default_objects.h>
#include <sblib/timeout.h>
#include "rm_com.h"
#include "rm_app.h"

// Digital pin for LED
//#define PIO_LED PIO2_0


// Place at own section
volatile const char __attribute__((used)) APP_VERSION[20] = "!AVP!@:S_RM_H6 1.01";

// disable optimization seems to be the only way to ensure that this is not being removed by the linker
// to keep the variable, we need to declar a function that uses it
// alternatively, the link script may be modified by adding KEEP to the section
volatile const char * __attribute__((optimize("O0"))) getAppVersion()
{
    return APP_VERSION;
}


/**
 * Application setup
 */
void setup()
{
    volatile const char * v = getAppVersion();		// Ensure APP ID is not removed by linker
    bcu.begin(0x004C, 0x03F2, 0x24); 		//Herstellercode 0x004C = Robert Bosch, Devicetype 1010 (0x03F2), Version 2.4

    //pinMode(PIO_LED, OUTPUT);
    //digitalWrite(PIO_LED, 1);

    // Handle power-up delay
    //Timeout delay;
    //delay.start(userEeprom.addrTab[0]*20); //aus LPC922 Rauchmelder übernommen

	pinMode(RM_ACTIVITY_PIN, INPUT);		// Status RM Bodenplatte, no pullup or pulldown configured at this pin to not affect the Rauchmelder

    initApplication();
}

/**
 * The application's main.
 */
void loop()
{
    int objno;

  	rm_recv_byte();

	if (!answerWait)
		process_alarm_stats();

	if (!answerWait)
    	process_objs();

	// Empfangenes Telegramm bearbeiten, aber nur wenn wir gerade nichts
	// vom Rauchmelder empfangen.


    // Handle updated communication objects
    while ((objno = nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }


    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}
