/*
 *  app_main.cpp - The application's main.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_out8.h"
#include "com_objs.h"

#include <sblib/eib.h>
#include <sblib/eib/sblib_default_objects.h>

#include "outputs.h"
#include "bus_voltage.h"
#include "app_nov_settings.h"

#include "MemMapperMod.h"

// TODO move this all to separate source
// from out-cs-bim112/app_main.cpp
/*
 * Der MemMapper bekommt einen 256 Byte Bereich ab 0xEA00, knapp unterhalb des UserMemory-Speicherbereichs ab 0xF000.
 * - für die Systemzustände. Diese werden bei Busspannungsausfall und Neustart abgespeichert.
 */
MemMapperMod memMapper(0xea00, 0x100);

void RecallAppData()
{
    byte* StoragePtr;
    StoragePtr = memMapper.memoryPtr(0, false);
    for (unsigned int i = 0; i < sizeof(AppSavedSettings); i++)
    {
        byte tmp = *StoragePtr++; // TODO remove tmp
        AppSavedSettings.AppValues[i] = tmp;
    }
    // TODO implemenent crc-check
}

void StoreApplData()
{
    byte* StoragePtr;
    // Kann der Mapper überhaupt die Seite 0 mappen? Checken!
    memMapper.writeMem(0, 0); // writeMem() aktiviert die passende Speicherseite, entgegen zu memoryPtr()
    StoragePtr = memMapper.memoryPtr(0, false);

#ifdef DEBUG
    AppSavedSettings.testBusRestartCounter++; // TODO remove after testing
#endif

    for (unsigned int i = 0; i < sizeof(AppSavedSettings); i++)
    {
        byte tmp = AppSavedSettings.AppValues[i]; // TODO remove tmp
        *StoragePtr++ = tmp;
    }
    // TODO implemenent crc-check and save in flash

    memMapper.doFlash(); // Erase time for one sector is 100 ms ± 5%. Programming time for one block of 256
                         // bytes is 1 ms ± 5%.
                         // see manual page 407
}

/* is used nowhere. Why its here ?
extern "C" const char APP_VERSION[13] = "O08.10  1.00"; // TODO move this also to config.h

const char * getAppVersion()
{
    return APP_VERSION;
}
*/

ObjectValues& objectValues = *(ObjectValues*) (userRamData + UR_COM_OBJ_VALUE0);

//#define IO_TEST

/*
 * Initialize the application.
 */
void setup()
{
#ifdef DEBUG
    // first set pin mode for Info & Run LED
    pinMode(PIN_INFO, OUTPUT); // this also sets pin to high/true
    pinMode(PIN_RUN, OUTPUT); // this also sets pin to high/true
    // then set value
    digitalWrite(PIN_INFO, 0);
    digitalWrite(PIN_RUN, 1);
#endif

    // volatile const char * v = getAppVersion();
    bcu.begin(MANUFACTURER, DEVICETYPE, APPVERSION);

    _bcu.setMemMapper((MemMapper *)&memMapper); // Der BCU wird hier der modifizierte MemMapper bekanntgemacht
    memMapper.addRange(0x0, 0x100); // Zum Abspeichern/Laden des Systemzustands
    RecallAppData(); // load custom app settings

    // enable bus voltage monitoring on PIO1_11 & AD7 with 1.94V threshold
    vBus.enableBusVRefMonitoring(PIN_VBUS, VBUS_AD_CHANNEL, VBUS_THRESHOLD);

    // Configure the output pins
    for (int channel = 0; channel < NO_OF_OUTPUTS; ++channel)
    {
        pinMode(outputPins[channel], OUTPUT);
        digitalWrite(outputPins[channel], 0);
    }

#ifdef IO_TEST
#ifdef HAND_ACTUATION
    for (unsigned int i = 0; i < NO_OF_CHANNELS; i++)
    {
        digitalWrite(handPins[i], 0);
        pinMode(handPins[i], OUTPUT);
    }
#endif
    for (unsigned int i = 0; i < NO_OF_OUTPUTS; i++)
    {
        digitalWrite(outputPins[i], 1);
#ifdef HAND_ACTUATION
        if (i < NO_OF_CHANNELS)
            digitalWrite(handPins[i], 1);
#endif
        delay(500);
        digitalWrite(outputPins[i], 0);
#ifdef HAND_ACTUATION
        if (i < NO_OF_CHANNELS)
            digitalWrite(handPins[i], 0);
#endif
        delay(500);
    }
#endif
#ifndef BI_STABLE
    //pinMode(PIN_IO11, OUTPUT);
    //digitalWrite(PIN_IO11, 1);
#endif
    initApplication();
#ifndef BI_STABLE
#ifdef ZERO_DETECT
    pinInterruptMode(PIO_SDA, INTERRUPT_EDGE_FALLING | INTERRUPT_ENABLED);
    enableInterrupt(EINT0_IRQn);
    pinEnableInterrupt(PIO_SDA);
#endif
#endif
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
    // check if any of the timeouts for an output has expire and react on them
    checkTimeouts();

    // check the bus voltage, should be done before waitForInterrupt()
    vBus.checkPeriodic();
    int tmp = vBus.valuemV();
    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
    {
        waitForInterrupt();
    }
}

/*
 * will be called by the bus_voltage.h ISR which handles the ADC interrupt for the bus voltage.
 */
void BusVoltageFail()
{
#ifdef HAND_ACTUATION
    // switch off all Handactuation LED to save some power
    handAct.setallLedState(false);
#endif

#ifdef DEBUG
    digitalWrite(PIN_RUN, 0); // switch RUN-LED to save some power
    digitalWrite(PIN_INFO, 1);
#endif

    StoreApplData(); // write application settings to flash

#ifdef DEBUG
    digitalWrite(PIN_INFO, 0);
#endif
}

/*
 * will be called by the bus_voltage.h ISR which handles the ADC interrupt for the bus voltage.
 */
void BusVoltageReturn()
{
    RecallAppData();
    initApplication();
#ifdef DEBUG
    digitalWrite(PIN_RUN, 1); // switch RUN-LED ON
#endif
}

