/*
 *  app_main.cpp - The application's main.
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <config.h>
#include <sblib/eibMASK0701.h>
#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>
#include "app-out8-dimmer.h"
#include <sblib/serial.h>
#include <sblib/mem_mapper.h>

// Hardware version. Must match the product_serial_number in the VD's table hw_product
const HardwareVersion hardwareVersion[] =
{ {8, 0xADF0, { 0x00, 0x00, 0x40, 0x00, 0x00, 0x00 }}
};

const HardwareVersion * currentVersion;

APP_VERSION("SBout8di", "0", "10");

Timeout timeout;
/*
AbsSegment1: 0x4000 (16384) Size 0x01FF (511)   End 0x41ff (16895)
AbsSegment2: 0x4200 (16896) Size 0x01FD (509)   End 0x43fd (17405)
AbsSegment3: 0x4400 (17408) Size 0x7200 (29184) End 0xB600 (46592)
*/
MemMapper memMapper(0xb000, 0x3f00, false);

/**
 * Initialize the application.
 */
BcuBase* setup()
{
#if defined (__LPC11XX__)
    serial.setRxPin(PIO2_7); // 4TE ID-SEL
    serial.setTxPin(PIO2_8); // 4TE ID-SEL
    // serial.setRxPin(PIO3_1); // GNAX and TS_ARM
    // serial.setTxPin(PIO3_0); // GNAX and TS_ARM
    serial.begin(115200);
    serial.println("out8-dimmer-bim112 online");
#elif defined (__LPC11UXX__)
#   error "set correct serial pins for LPCUxxx" // TODO set correct serial pins for LPCUxxx
#else
#   error "unknown cpu"
#endif

    // MASK0701 userEeprom start 0x3f00 end 0x4aff, we need more space !!11!
    if (memMapper.addRange(0x4b00, 0x3400) != MEM_MAPPER_SUCCESS)
    {
        fatalError();
    }

    // need this to "capture" end of AbsSegment1 otherwise PropertiesMASK0701::handleAllocAbsDataSegment will fail
    if (memMapper.addRange(0xad00, 0xa00) != MEM_MAPPER_SUCCESS)
    {
        fatalError();
    }


#if defined (__LPC11XX__)
#   if defined(__LPC11XX_IO16__)
        // GNAX2 board
        bcu.setProgPin(PIO2_11); // XIO23
        bcu.setProgPinInverted(false);
        bcu.setRxPin(PIO1_8);
        bcu.setTxPin(PIO1_9);
#   endif // __LPC11XX_IO16__
#elif defined (__LPC11UXX__)
#   error "set correct bcu-pins for LPCUxxx" // TODO set correct bcu-pins for LPCUxxx
#else
#   error "unknown cpu"
#endif // __LPC11XX__
    currentVersion = &hardwareVersion[0];
    bcu.comObjects->objectEndian(LITTLE_ENDIAN);  ///\todo this is a test, not sure it's really little endian
    bcu.begin(0x0002, 0xa045, 0x0012); // ABB SD/S8.16.1 Switch/Dim Actuator, 8-fold, MDRC
    bcu.setMemMapper(&memMapper);
    bcu.setHardwareType(currentVersion->hardwareVersion, sizeof(currentVersion->hardwareVersion));

    pinMode(PIN_INFO, OUTPUT);	// Info LED
    pinMode(PIN_RUN,  OUTPUT);	// Run LED
    initApplication();
	timeout.start(1);
	return (&bcu);
}

/**
 * The main processing loop.
 */
void loop()
{
    int objno;
    // Handle updated communication objects
    while ((objno = bcu.comObjects->nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }
    // check the periodic function of the application
    checkPeriodicFuntions();

    // Sleep up to 1 millisecond if there is nothing to do
    waitForInterrupt();
    if (timeout.started() && timeout.expired())
    {
    	timeout.start(1000);
    	digitalWrite(PIN_INFO, !digitalRead(PIN_INFO));
    }
}

void loop_noapp()
{
    if (!bcu.programmingMode())
    {
        HandActuation::testIO(&handPins[0],  NO_OF_HAND_PINS, BLINK_TIME);
    }
    waitForInterrupt();
}

void loop_test(void)
{
    static int i = -1;
    if (i == -1)
    {
        pinMode(PIN_PWM, OUTPUT);
        digitalWrite(PIN_PWM, 0);
        for (i = 0; i < NO_OF_OUTPUTS; i++)
        {
            pinMode(outputPins[i], OUTPUT);
            digitalWrite(outputPins[i], 0);

            pinMode(handPins[i], OUTPUT);
            digitalWrite(handPins[i], 0);

        }
        i = 0;
    }
    else if (i < (2 * NO_OF_OUTPUTS))
    {
        if (timeout.expired ())
        {
            unsigned int n = i >> 1;
            timeout.start  (500);
            digitalWrite (outputPins[n], !(i & 0x01));

            digitalWrite (handPins[n], !(i & 0x01));

            i++;
        }
    }
}
