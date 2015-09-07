/*
 *  app_main.cpp - The application's main.
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

//#define NEW_LIB

#include <sblib/eib.h>
#ifdef NEW_LIB
#include <sblib/eib/sblib_default_objects.h>
#define BCU_ACCESS(x) bcu->x
#else
#define BCU_ACCESS(x) bcu.x
#endif

#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>
#include <sblib/timeout.h>
#include <string.h>
#include "channel.h"
#include "led-controller.h"

extern "C" const char APP_VERSION[13] = "LED 0.1";

const char * getAppVersion()
{
    return APP_VERSION;
}


// Hardware version. Must match the product_serial_number in the VD's table hw_product
const unsigned char hardwareVersion[] =
{ 0x00, 0x00, 0x00, 0x00, 0x02, 0x40};

/* initialization of the application software */
void initApplication(void);

/* called when a com object has been changed from outside */
void objectUpdated(unsigned int objno);

/* call in the main loop to handle the periodic functions */
void checkPeriodicFuntions(void);

static Channel channels[4];

/*
 * Initialize the application.
 */
void setup()
{
    volatile char v = getAppVersion()[0];
    v++;
    BCU_ACCESS(begin)(131, hardwareVersion[5], 0x13);  // we are a MDT weather station, version 1.3
    memcpy(userEeprom.order, hardwareVersion, sizeof(hardwareVersion));

    pinMode(PIN_INFO, OUTPUT);	// Info LED
    pinMode(PIN_RUN,  OUTPUT);	// Run LED
    if (BCU_ACCESS(applicationRunning) ())
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
    // XXX setup the PWM frequency
    unsigned int pwmFreq = userEeprom.getUInt8 (0x476C) * 1000;
    timer16_0.prescaler(pwmFreq);

}

void objectUpdated(unsigned int objno)
{
}

void checkPeriodicFuntions(void)
{
    if (userEeprom.getUInt8 (0x4764) == 1)
    {   // relay output is controlled by the 4 channels
        unsigned int value = channels[0].isOn() + channels[1].isOn() + channels[2].isOn() + channels[3].isOn();
        objectSetValue(COM_OBJ_RELAI_SWITCH, value > 0);
    }
    // update the relay output state
    digitalWrite(RELAY_OUTPUT, objectRead(COM_OBJ_RELAI_SWITCH));
}

