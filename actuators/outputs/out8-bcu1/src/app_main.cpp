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

#ifdef BUSFAIL
#    include "bus_voltage.h"
#    include "app_nov_settings.h"
#endif


#ifdef BUSFAIL
    typedef struct
    {
        unsigned char relaisstate;         // current relays state
        unsigned char handactuationstate;  // current hand actuation state
#       ifdef DEBUG
            unsigned char testBusRestartCounter; //TODO rename or remove after testing
#       endif
    } ApplicationData;

    NonVolatileSetting AppNovSetting(0xEA00, 0x100, sizeof(ApplicationData));  // flash-storage for application relevant parameters
    ApplicationData AppData;
#endif



ObjectValues& objectValues = *(ObjectValues*) (userRamData + UR_COM_OBJ_VALUE0);

/*
 * simple IO test
 */
void ioTest()
{
#ifdef IO_TEST
     // setup LED's
     const int togglePausems = 500;
#    ifdef HAND_ACTUATION
        for (unsigned int i = 0; i < NO_OF_CHANNELS; i++)
        {
            digitalWrite(handPins[i], 0);
            pinMode(handPins[i], OUTPUT);
        }
#    endif /* HAND_ACTUATION */

     // toggle every channel/LED with a delay of 500ms
     for (unsigned int i = 0; i < NO_OF_OUTPUTS; i++)
     {
         digitalWrite(outputPins[i], 1);   // relay high
#ifdef HAND_ACTUATION
         if (i < NO_OF_CHANNELS)
             digitalWrite(handPins[i], 1); // LED high
#endif /* HAND_ACTUATION */
         delay(togglePausems);
         digitalWrite(outputPins[i], 0);   // relay low
#ifdef HAND_ACTUATION
         if (i < NO_OF_CHANNELS)
             digitalWrite(handPins[i], 0); // LED low
#endif /* HAND_ACTUATION */
         delay(togglePausems);
     }
#endif
}

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

    bcu.begin(MANUFACTURER, DEVICETYPE, APPVERSION);

#ifdef BUSFAIL
    if (!AppNovSetting.RecallAppData((unsigned char*)&AppData, sizeof(ApplicationData))) // load custom app settings
    {
#   ifdef DEBUG
        digitalWrite(PIN_INFO, 1);
        delay(1000);
        digitalWrite(PIN_INFO, 0);
#   endif
    }
    // enable bus voltage monitoring on PIO1_11 & AD7 with 1.94V threshold
    vBus.enableBusVRefMonitoring(PIN_VBUS, VBUS_AD_CHANNEL, VBUS_THRESHOLD);
#endif

    // Configure the output pins
    for (int channel = 0; channel < NO_OF_OUTPUTS; ++channel)
    {
        pinMode(outputPins[channel], OUTPUT);
        digitalWrite(outputPins[channel], 0);
    }

    ioTest();

#ifndef BI_STABLE
    //pinMode(PIN_IO11, OUTPUT);
    //digitalWrite(PIN_IO11, 1);
#endif

#ifndef BI_STABLE
#   ifdef ZERO_DETECT
       pinInterruptMode(PIO_SDA, INTERRUPT_EDGE_FALLING | INTERRUPT_ENABLED);
       enableInterrupt(EINT0_IRQn);
       pinEnableInterrupt(PIO_SDA);
#   endif
#endif

#ifdef BUSFAIL
    initApplication(AppData.relaisstate);
#else
    initApplication();
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
#ifdef BUSFAIL
    // check the bus voltage, should be done before waitForInterrupt()
    vBus.checkPeriodic();
#endif
    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
    {
        waitForInterrupt();
    }
}

#ifdef BUSFAIL

void ResetDefaultApplicationData()
{
    AppData.relaisstate = 0x00;
    AppData.handactuationstate = 0x00;
}
/*
 * will be called by the bus_voltage.h ISR which handles the ADC interrupt for the bus voltage.
 */
void BusVoltageFail()
{
#ifdef HAND_ACTUATION
    // switch all Handactuation LEDs off, to save some power
    handAct.setallLedState(false);
#endif

#ifdef DEBUG
    AppData.testBusRestartCounter++;
    digitalWrite(PIN_RUN, 0); // switch RUN-LED off, to save some power
    digitalWrite(PIN_INFO, 1);
#endif

    AppData.relaisstate = getRelaysState();
    AppNovSetting.StoreApplData((unsigned char*)&AppData, sizeof(ApplicationData)); // write application settings to flash

#ifdef DEBUG
    digitalWrite(PIN_INFO, 0);
#endif
}

/*
 * will be called by the bus_voltage.h ISR which handles the ADC interrupt for the bus voltage.
 */
void BusVoltageReturn()
{
    if (!AppNovSetting.RecallAppData((unsigned char*)&AppData, sizeof(AppData))) // load custom app settings
    {
        // load default values
        ResetDefaultApplicationData();
#ifdef DEBUG
        digitalWrite(PIN_INFO, 1);
        delay(1000);
        digitalWrite(PIN_INFO, 1);
#endif
    }

#ifdef BUSFAIL
    initApplication(AppData.relaisstate);
#else
    initApplication();
#endif

#ifdef DEBUG
    digitalWrite(PIN_RUN, 1); // switch RUN-LED ON
#endif
}
#endif /* BUSFAIL */
