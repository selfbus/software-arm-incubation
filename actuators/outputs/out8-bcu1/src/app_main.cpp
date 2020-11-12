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
#include <sblib/io_pin_names.h>
#include "outputs.h"
#include "bus_voltage.h"

#include "MemMapperMod.h"

byte testBusRestartCounter; //TODO remove after testing

// TODO move this all to separate source
// from out-cs-bim112/app_main.cpp
/*
 * Der MemMapper bekommt einen 1kB Bereich ab 0xEA00, knapp unterhalb des UserMemory-Speicherbereichs ab 0xF000.
 * Damit lassen sich 3 Pages (je 256Byte) (und die allocTable die MemMappers) unterbringen.
 * Benötigt werden zwei Pages:
 * - für den Konfigurationsspeicher jenseits von 0x4B00. Für die SbLib endet der Konfigurationspeicher dort,
 *   unser Vorbild legt dort jedoch noch einige allgemeine Optionen ab.
 * - für die Systemzustände. Diese werden bei Busspannungsausfall und Neustart abgespeichert.
 */
MemMapperMod memMapper(0xea00, 0x400);

void RecallAppData()
{
    byte* StoragePtr;
    StoragePtr = memMapper.memoryPtr(0, false);
    testBusRestartCounter  =  *StoragePtr++;
}

void StoreApplData()
{
    digitalWrite(PIN_INFO, 1);
    byte* StoragePtr;
    // Kann der Mapper überhaupt die Seite 0 mappen? Checken!
    memMapper.writeMem(0, 0); // writeMem() aktiviert die passende Speicherseite, entgegen zu memoryPtr()
    StoragePtr = memMapper.memoryPtr(0, false);
    *StoragePtr++ = testBusRestartCounter;
    memMapper.doFlash(); // Erase time for one sector is 100 ms ± 5%. Programming time for one block of 256
                         // bytes is 1 ms ± 5%.
                         // see manual page 407
    digitalWrite(PIN_INFO, 0);
}

extern "C" const char APP_VERSION[13] = "O08.10  1.00";

const char * getAppVersion()
{
    return APP_VERSION;
}

// Output pins
#ifdef BI_STABLE
//RESET Pin SET Pin
const int outputPins[NO_OF_OUTPUTS] =
#if 1
// Pinbelegung für ???
{ PIN_IO2,  PIN_IO3 //  1,  2
, PIN_IO5,  PIO_SDA //  3,  4
, PIN_PWM,  PIN_APRG //  5,  6
, PIN_IO1,  PIN_IO4 //  7,  8

, PIN_TX,   PIN_IO11   // 15, 16
, PIN_IO10, PIN_RX // 13, 14
, PIN_IO14, PIN_IO15 // 11, 12
, PIN_IO9,  PIN_IO13 //  9, 10
};
#endif
#if 0
//Pinbelegung für out8_16A-V2.3 (bistabile Relais, separate Kanäle ohne Zusammenlegung von Kontakten)
{ PIN_IO5,  PIN_IO7  // Kanal 1
, PIN_IO4,  PIN_IO3  // Kanal 2
, PIN_PWM,  PIN_APRG // Kanal 3
, PIN_IO2,  PIN_IO1  // Kanal 4

, PIN_IO10, PIN_RX   // Kanal 5
, PIN_TX,   PIN_IO11 // Kanal 6
, PIN_IO14, PIN_IO15 // Kanal 7
, PIN_IO9,  PIN_IO13 // Kanal 8
};
#endif
#if 0
// Pinbelegung für out8_16A-V2.2 (bistabile Relais, Eingänge von jeweils 2 Kanälen zusammen)
{ PIN_IO1,  PIN_IO4  // Kanal 1
, PIN_PWM,  PIN_APRG // Kanal 2
, PIN_IO5,  PIN_IO7  // Kanal 3
, PIN_IO2,  PIN_IO3  // Kanal 4

, PIN_TX,   PIN_IO11 // Kanal 5
, PIN_IO10, PIN_RX   // Kanal 6
, PIN_IO14, PIN_IO15 // Kanal 7
, PIN_IO9,  PIN_IO13 // Kanal 8
};
#endif
#if 0
// Pinbelegung für ???
{ PIN_IO3,  PIN_IO2 //  1,  2
, PIN_IO12, PIN_IO5 //  3,  4
, PIN_APRG, PIN_PWM //  5,  6
, PIN_IO4,  PIN_IO1 //  7,  8

, PIN_IO13, PIN_IO9 //  9, 10
, PIN_IO15, PIN_IO14 // 11, 12
, PIN_RX,   PIN_IO10 // 13, 14
, PIN_IO11, PIN_TX   // 15, 16
};
#endif
#else
const int outputPins[NO_OF_OUTPUTS] =
    { PIN_IO1, PIN_IO2, PIN_IO3, PIN_IO4, PIN_IO5, PIN_IO6, PIN_IO7, PIN_IO8 };
#endif

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

    volatile const char * v = getAppVersion();
    bcu.begin(4, 0x2060, 1); // We are a "Jung 2138.10" device, version 0.1

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
#endif
    testBusRestartCounter++;
    StoreApplData();
}

/*
 * will be called by the bus_voltage.h ISR which handles the ADC interrupt for the bus voltage.
 */
void BusVoltageReturn()
{
    RecallAppData();
#ifdef DEBUG
    digitalWrite(PIN_RUN, 1); // switch RUN-LED ON
#endif
}

