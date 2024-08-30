/*
 *  OneWire Gateway Applikation for LPC1115
 *
 *  Copyright (C) 2021-2024 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

/*
 * This Software is used by a board with 2 or 4 DS2482 OneWire Controller
 * These Controllers are connected to the MCU over I2C
 */
#include <sblib/eibMASK0701.h>
#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>

extern "C" const char APP_VERSION[9] = "OWGW 0.1";
#include "config.h"
#include "app-OneWireGateway.h"

MASK0701 bcu = MASK0701();

/*
 * Initialize the application.
 */
BcuBase* setup() {
	bcu.begin(MANUFACTURER, DEVICETYPE, APPVERSION);

	initApplication();

	return &bcu;
}

/*
 * The main processing loop.
 */
void loop() {
	int objno;
	// Handle updated communication objects
	while ((objno = bcu.comObjects->nextUpdatedObject()) >= 0) {
		objectUpdated(objno);
	}
	// check the periodic function of the application
	checkPeriodicFuntions();
}

/**
 * The processing loop while no KNX-application is loaded
 */
void loop_noapp()
{

}
