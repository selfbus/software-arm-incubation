/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/bcu_base.h>
#include <sblib/eibMASK0701.h>
#include <sblib/serial.h>
#include <cstring>
#include "Hoermann.h"
#include "HoermannState.h"

#include <HelperFunctions.h>

APP_VERSION("Hoermann", "0", "1"); // Don't forget to also change the build-variable sw_version

MASK0701 bcu = MASK0701();

// Digital pin for LED
#define PIO_LED PIO2_0

const unsigned char hardwareVersion[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x48 };

Hoermann garageDoor;
HoermannState doorState;

const uint8_t GO_IS_OPEN = 1;
const uint8_t GO_IS_CLOSED = 2;
const uint8_t GO_OPTION_RELAY = 3;
const uint8_t GO_LIGHT_RELAY = 4;
const uint8_t GO_HAS_ERROR = 5;
const uint8_t GO_DIRECTION_DOWN = 6;
const uint8_t GO_IS_MOVING = 7;
const uint8_t GO_IS_VENTING = 8;
const uint8_t GO_HAS_PRE_WARNING = 9;
const uint8_t GO_OPENCLOSE = 10;
const uint8_t GO_STOP = 11;
const uint8_t GO_LIGHT = 12;

/**
 * Application setup
 */
BcuBase* setup()
{
    bcu.setHardwareType(hardwareVersion, sizeof(hardwareVersion));
    bcu.begin(0x13A, 0x02, 0x01); // Manufacturer name "Not assigned", app-id 0x02, version 0.01

    pinMode(PIO_LED, OUTPUT);
    digitalWrite(PIO_LED, 1);

    HelperFunctions::setFlagsTablePtr(&bcu, 0x6C5);
    uint16_t objRamPointer = 0x5FC;

    HelperFunctions::setComObjPtr(&bcu, GO_IS_OPEN, ComType::BIT_1, objRamPointer);
    HelperFunctions::setComObjPtr(&bcu, GO_IS_CLOSED, ComType::BIT_1, objRamPointer);
    HelperFunctions::setComObjPtr(&bcu, GO_OPTION_RELAY, ComType::BIT_1, objRamPointer);
    HelperFunctions::setComObjPtr(&bcu, GO_LIGHT_RELAY, ComType::BIT_1, objRamPointer);
    HelperFunctions::setComObjPtr(&bcu, GO_HAS_ERROR, ComType::BIT_1, objRamPointer);
    HelperFunctions::setComObjPtr(&bcu, GO_DIRECTION_DOWN, ComType::BIT_1, objRamPointer);
    HelperFunctions::setComObjPtr(&bcu, GO_IS_MOVING, ComType::BIT_1, objRamPointer);
    HelperFunctions::setComObjPtr(&bcu, GO_IS_VENTING, ComType::BIT_1, objRamPointer);
    HelperFunctions::setComObjPtr(&bcu, GO_HAS_PRE_WARNING, ComType::BIT_1, objRamPointer);
    HelperFunctions::setComObjPtr(&bcu, GO_OPENCLOSE, ComType::BIT_1, objRamPointer);
    HelperFunctions::setComObjPtr(&bcu, GO_STOP, ComType::BIT_1, objRamPointer);
    HelperFunctions::setComObjPtr(&bcu, GO_LIGHT, ComType::BIT_1, objRamPointer);

    return &bcu;
}

/**
 * The application's main.
 */
void loop()
{
    // Handle updated communication objects
    int objNo = -1;

    while ((objNo = bcu.comObjects->nextUpdatedObject()) >= 0)
    {
    	switch (objNo)
    	{
    	case 10: // open/close
			if (garageDoor.state.moving)
			{
				garageDoor.stop();
			}
    		if (bcu.comObjects->objectRead(objNo))
    		{
    			if (!garageDoor.state.moving || garageDoor.state.directionDown)
    			{
					garageDoor.open();
    			}
    		}
    		else
    		{
    			if (!garageDoor.state.moving || !garageDoor.state.directionDown)
    			{
					garageDoor.close();
    			}
    		}
    		break;
    	case 11: // stop
    		garageDoor.stop();
    		break;
    	case 12: // light
    		garageDoor.light(bcu.comObjects->objectRead(objNo) != 0);
    		break;
    	}
    }

    garageDoor.loop();

	if (doorState.doorOpen != garageDoor.state.doorOpen)
	{
		bcu.comObjects->objectWrite(GO_IS_OPEN, garageDoor.state.doorOpen);
    }
	if (doorState.doorClosed != garageDoor.state.doorClosed)
	{
		bcu.comObjects->objectWrite(GO_IS_CLOSED, garageDoor.state.doorClosed);
    }
	if (doorState.optionRelay != garageDoor.state.optionRelay)
	{
		bcu.comObjects->objectWrite(GO_OPTION_RELAY, garageDoor.state.optionRelay);
    }
	if (doorState.lightRelay != garageDoor.state.lightRelay)
	{
		bcu.comObjects->objectWrite(GO_LIGHT_RELAY, garageDoor.state.lightRelay);
    }
	if (doorState.error != garageDoor.state.error)
	{
		bcu.comObjects->objectWrite(GO_HAS_ERROR, garageDoor.state.error);
    }
	if (doorState.directionDown != garageDoor.state.directionDown)
	{
		bcu.comObjects->objectWrite(GO_DIRECTION_DOWN, garageDoor.state.directionDown);
    }
	if (doorState.moving != garageDoor.state.moving)
	{
		bcu.comObjects->objectWrite(GO_IS_MOVING, garageDoor.state.moving);
    }
	if (doorState.ventingPos != garageDoor.state.ventingPos)
	{
		bcu.comObjects->objectWrite(GO_IS_VENTING, garageDoor.state.ventingPos);
    }
	if (doorState.preWarning != garageDoor.state.preWarning)
	{
		bcu.comObjects->objectWrite(GO_HAS_PRE_WARNING, garageDoor.state.preWarning);
    }
	garageDoor.state.copyTo(doorState);

    // Sleep up to 1 millisecond if there is nothing to do
	waitForInterrupt();
}

/**
 * The processing loop while no KNX-application is loaded
 */
void loop_noapp()
{

}
