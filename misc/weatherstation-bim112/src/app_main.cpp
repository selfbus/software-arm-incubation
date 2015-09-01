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
#include <sblib/eib/sblib_default_objects.h>
#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>
#include <sblib/timeout.h>
#include <string.h>
#include "weatherstation.h"
#include "brightness_sensor.h"

extern "C" const char APP_VERSION[13] = "WS 0.1";

const char * getAppVersion()
{
    return APP_VERSION;
}


// Hardware version. Must match the product_serial_number in the VD's table hw_product
const unsigned char hardwareVersion[] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D };

/* initialization of the application software */
void initApplication(void);

/* called when a com object has been changed from outside */
void objectUpdated(unsigned int objno);

/* call in the main loop to handle the periodic functions */
void checkPeriodicFuntions(void);

static Timeout monitorDelay;
static unsigned int monitorTime;
Brightness_Sensor brightness[3];

/*
 * Initialize the application.
 */

void setup()
{
    volatile char v = getAppVersion()[0];
    v++;
    bcu->begin(131, hardwareVersion[5], 0x13);  // we are a MDT weather station, version 1.3
    memcpy(userEeprom.order, hardwareVersion, sizeof(hardwareVersion));

    pinMode(PIN_INFO, OUTPUT);	// Info LED
    pinMode(PIN_RUN,  OUTPUT);	// Run LED
    if (bcu->applicationRunning())
        initApplication();
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
        objectUpdated(objno);
    }
    // check the periodic function of the application
    checkPeriodicFuntions();

    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}

void initApplication(void)
{
	monitorTime = userEeprom.getUInt8(0x4507);
	if (monitorTime < 40)
	{   // value from the EEPROM is in hours -> convert them into milliseconds
		monitorTime *= 3600000;
	}
	else
	{   // value is in 0.25min -> convert them into milliseconds
		monitorTime *= 15000;
	}
	if (monitorTime)
		monitorDelay.start (100); // send the first alive signal after 100ms

	for (unsigned int i = 0; i < 3; i++)
	{
	    if (userEeprom.getUInt8(0x450B + i))
	        brightness[i].Initialize(i);
	}
}

void objectUpdated(unsigned int objno)
{
    unsigned int channel = objno / 17;
    if (channel < 3)
    {
        brightness[channel].objectUpdated(objno);
    }
}

void checkPeriodicFuntions(void)
{
	// handle the alive status message
	if (monitorTime && monitorDelay.expired())
	{
		objectWrite(COM_OBJ_STATUS, 1);
		monitorDelay.start (monitorTime);
	}

}

