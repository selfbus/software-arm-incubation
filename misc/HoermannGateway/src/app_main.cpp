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
#include "config.h"

#include <HelperFunctions.h>

APP_VERSION("Hoermann", "0", "01"); // Don't forget to also change the build-variable sw_version

MASK0701 bcu = MASK0701();

const unsigned char hardwareVersion[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x48 };

Hoermann garageDoor;
HoermannState doorState;

enum GroupObjectNumber : uint8_t
{
    GO_IS_OPEN = 1,
    GO_IS_CLOSED = 2,
    GO_OPTION_RELAY = 3,
    GO_LIGHT_RELAY = 4,
    GO_HAS_ERROR = 5,
    GO_DIRECTION_DOWN = 6,
    GO_IS_MOVING = 7,
    GO_IS_VENTING = 8,
    GO_HAS_PRE_WARNING = 9,
    GO_OPENCLOSE = 10,
    GO_STOP = 11,
    GO_LIGHT = 12
};

/**
 * Application setup
 */
BcuBase* setup()
{
    pinMode(PIO_LED_1, OUTPUT);
    pinMode(PIO_LED_2, OUTPUT);
    digitalWrite(PIO_LED_1, true);

    bcu.setHardwareType(hardwareVersion, sizeof(hardwareVersion));
    bcu.begin(0x13A, 0x02, 0x01); // Manufacturer name "Not assigned", app-id 0x02, version 0.01

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

    digitalWrite(PIO_LED_2, true);
    return &bcu;
}

void sendGroupObjectIfChanged(const GroupObjectNumber groupObjectNumber, const bool currentValue, const bool newValue)
{
    if (currentValue != newValue)
    {
        return;
    }

    bcu.comObjects->objectWrite(groupObjectNumber, newValue);
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
            case GO_OPENCLOSE:
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

            case GO_STOP:
                garageDoor.stop();
                break;

            case GO_LIGHT:
                garageDoor.light(bcu.comObjects->objectRead(objNo) != 0);
                break;

            default:
                ///\todo failHardInDebug();
                break;
        }
    }

    garageDoor.loop();

    sendGroupObjectIfChanged(GO_IS_OPEN, doorState.doorOpen, garageDoor.state.doorOpen);
    sendGroupObjectIfChanged(GO_IS_CLOSED, doorState.doorClosed, garageDoor.state.doorClosed);
    sendGroupObjectIfChanged(GO_OPTION_RELAY, doorState.optionRelay, garageDoor.state.optionRelay);
    sendGroupObjectIfChanged(GO_LIGHT_RELAY, doorState.lightRelay, garageDoor.state.lightRelay);
    sendGroupObjectIfChanged(GO_HAS_ERROR, doorState.error, garageDoor.state.error);
    sendGroupObjectIfChanged(GO_DIRECTION_DOWN, doorState.directionDown, garageDoor.state.directionDown);
    sendGroupObjectIfChanged(GO_IS_MOVING, doorState.moving, garageDoor.state.moving);
    sendGroupObjectIfChanged(GO_IS_VENTING, doorState.ventingPos, garageDoor.state.ventingPos);
    sendGroupObjectIfChanged(GO_HAS_PRE_WARNING, doorState.preWarning, garageDoor.state.preWarning);

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
