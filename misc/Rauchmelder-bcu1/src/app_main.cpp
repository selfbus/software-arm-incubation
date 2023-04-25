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

#include <sblib/timeout.h>
#include "rm_com.h"
#include "rm_app.h"

APP_VERSION("S_RM_H6 ", "2", "03");

/**
 * Application setup
 */
BcuBase* setup()
{
    bcu.begin(0x004C, 0x03F2, 0x24); 		//Herstellercode 0x004C = Robert Bosch, Devicetype 1010 (0x03F2), Version 2.4

    // Handle power-up delay
    //Timeout delay;
    //delay.start(userEeprom.addrTab[0]*20); //aus LPC922 Rauchmelder übernommen

    initApplication();
    return (&bcu);
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
    while ((objno = bcu.comObjects->nextUpdatedObject()) >= 0)
    {
       objectUpdated(objno);
    }


    // Sleep up to 1 millisecond if there is nothing to do
    if (bcu.bus->idle())
        waitForInterrupt();
}

/**
 * The loop while no application is running.
 */
void loop_noapp()
{
    rm_recv_byte(); // timer32_0 is still running, so we should read the received bytes
}
