/*
 *  app_main.cpp - The application's main.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/analog_pin.h>
#include "app_main.h"
#include "app_out8.h"
#include "com_objs.h"

#ifdef BUSFAIL
#   include <sblib/math.h>
#endif

#ifndef BI_STABLE
#   include "outputs.h"
#else
#   include "outputsBiStable.h"
#endif

#ifdef BUSFAIL
#    include "bus_voltage.h"
#    include "app_nov_settings.h"
#endif

#ifdef DEBUG_SERIAL
#    include <sblib/serial.h>
#endif


#ifdef BUSFAIL
    NonVolatileSetting AppNovSetting(0xEE00, 0x100);  // flash-storage for application relevant parameters
    ApplicationData AppData;
    AppCallback callback;
    AppUsrCallback usrCallback; ///\todo two callbacks? Optimize with busfail integration into the sblib.
#endif

APP_VERSION("O08.10  ", "5", "12");

/**
 * Simple IO test
 */
void ioTest()
{
#ifdef IO_TEST

#   ifdef BI_STABLE
        const int relaySwitchms = ON_DELAY;  // see inc/outputsBiStable.h for more info
        // check, maybe we need to wait 4s, cause PIO_SDA is pulled-up to high on ARM-reset causing the relay coil to drain the bus.
        const int countOutputPins = sizeof(outputPins)/sizeof(outputPins[0]);
        for (unsigned int i = 0; i < countOutputPins; i++)
        {
            if (outputPins[i] == PIO_SDA)
            {
                delay(4000);
                break;
            }
        }
#   else
        const int relaySwitchms = PWM_TIMEOUT;  // see inc/outputs.h for more info
#   endif

    // setup LED's
    const int togglePausems = 250;
#   ifndef HAND_ACTUATION
        const int disablePins[9] = {PIN_LT1, PIN_LT2, PIN_LT3, PIN_LT4, PIN_LT5, PIN_LT6, PIN_LT7, PIN_LT8, PIN_LT9};
        for (unsigned int i = 0; i < sizeof(disablePins)/sizeof(disablePins[0]); i++)
        {
            pinMode(disablePins[i], INPUT);
        }
#   endif /* HAND_ACTUATION */

    // all relay-pins off
    for (unsigned int i = 0; i < sizeof(outputPins)/sizeof(outputPins[0]); i++)
    {
        digitalWrite(outputPins[i], 0);
        pinMode(outputPins[i], OUTPUT);
    }

    // initialize relays for ioTest
    relays.begin(0x00, 0x00, NO_OF_CHANNELS);
#   ifdef HAND_ACTUATION
        HandActuation handActTestIO = HandActuation(&handPins[0], NO_OF_HAND_PINS, READBACK_PIN, BLINK_TIME);
        relays.setHandActuation(&handActTestIO);
#   endif
    relays.updateOutputs();
    delay(relaySwitchms);
    relays.checkPWM();
    delay(togglePausems);

    // toggle every channel & hand actuation LED with a little delay
    for (unsigned int i = 0; i < relays.channelCount(); i++)
    {
        relays.updateChannel(i, 1);     // relay high/set
        relays.updateOutput(i);         // also sets hand actuation led
        delay(relaySwitchms);
        relays.checkPWM();

        delay(togglePausems);

        relays.updateChannel(i, 0);     // relay low/reset
        relays.updateOutput(i);         // also sets hand actuation led
        delay(relaySwitchms);
        relays.checkPWM();
        delay(togglePausems);
    }
#endif /* IO_TEST */
}

void initSerial()
{
#ifdef DEBUG_SERIAL
    serial.setRxPin(PIO2_7);
    serial.setTxPin(PIO2_8);
    serial.begin(115200);
    serial.println("out8 serial debug started");
#endif
}

void printSerialBusVoltage(const unsigned int onEveryTickMs)
{
#if defined(DEBUG_SERIAL) && defined(BUSFAIL)
    if (millis() % onEveryTickMs == 0)
    {
        unsigned int valueAD = busVoltageMonitor.valueBusVoltageAD();
        unsigned int valueADConvertedTwice = callback.convertmVAD(callback.convertADmV(valueAD));
        unsigned int valuemV = callback.convertADmV(valueAD);
        int diff = int(valueAD - valueADConvertedTwice);
        serial.print("AD;");
        serial.print(valueAD);
        serial.print(";mV;");
        serial.print(valuemV);
        serial.print(";diff;");
        serial.print(diff);
        serial.println();
        delay(1); // makes sure that printSerialBusVoltage(...) is not called twice on the same SysTick
    }
#endif
}

void startBusVoltageMonitoring()
{
#ifdef BUSFAIL
    if (busVoltageMonitor.setup(VBUS_AD_PIN, VBUS_AD_CHANNEL, VBUS_ADC_SAMPLE_FREQ,
                                VBUS_THRESHOLD_FAILED, VBUS_THRESHOLD_RETURN,
                                VBUS_VOLTAGE_FAILTIME_MS, VBUS_VOLTAGE_RETURNTIME_MS,
                                &timer32_0, 0, &callback))
    {
        busVoltageMonitor.enable();
        while (busVoltageMonitor.busFailed())
        {
            delay(1);
#           ifdef DEBUG
                digitalWrite(PIN_RUN, !digitalRead(PIN_RUN));
#           endif
            printSerialBusVoltage(100);
        }
#       ifdef DEBUG
            digitalWrite(PIN_RUN, 1);
#       endif
    }
#endif
}

bool recallAppData()
{
    bool result = true;
#ifdef BUSFAIL
    result = AppNovSetting.RecallAppData((unsigned char*)&AppData, sizeof(ApplicationData)); // load custom application settings
    if (!result)
    {
#   ifdef DEBUG
        digitalWrite(PIN_INFO, 1);
        delay(1000);
        digitalWrite(PIN_INFO, 0);
#   endif
    }
#endif
    return result;
}

/**
 * Initialize the application.
 */
BcuBase* setup()
{
    // first set pin mode for Info & Run LED
    pinMode(PIN_INFO, OUTPUT); // this also sets pin to high/true
    pinMode(PIN_RUN, OUTPUT); // this also sets pin to high/true
    // then set values
    digitalWrite(PIN_INFO, 0);
#ifdef DEBUG
    digitalWrite(PIN_RUN, 1);
#endif

    // Configure the output pins
    relays.setupOutputs(&outputPins[0], NO_OF_OUTPUTS);

    initSerial();

    ioTest();
    bcu.begin(MANUFACTURER, DEVICETYPE, APPVERSION);
#ifdef BUSFAIL
    bcu.setUsrCallback((UsrCallback *)&usrCallback);
    startBusVoltageMonitoring(); // enable bus voltage monitoring
#endif

#ifdef DEBUG_SERIAL
    int physicalAddress = bus.ownAddress();
    serial.print("physical address: ", (physicalAddress >> 12) & 0x0F, DEC);
    serial.print(".", (physicalAddress >> 8) & 0x0F, DEC);
    serial.println(".", physicalAddress & 0xFF, DEC);
#endif

    recallAppData();

#ifndef BI_STABLE
#   ifdef ZERO_DETECT
       pinInterruptMode(PIO_SDA, INTERRUPT_EDGE_FALLING | INTERRUPT_ENABLED);
       enableInterrupt(EINT0_IRQn);
       pinEnableInterrupt(PIO_SDA);
#   endif
#endif

#ifdef BUSFAIL
    initApplication(AppData.relaysstate);
    startBusVoltageMonitoring(); // needs to be called again, because Release version is using analog_pin.h functions from sblib which break our ADC Interrupts
#else
    initApplication();
#endif
    return (&bcu);
}

/**
 * The main processing loop. Will be called by the Selfbus sblib main().
 */
void loop(void)
{
    int objno;
    // Handle updated communication objects
    while ((objno = bcu.comObjects->nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }
    // check if any of the timeouts for an output has expire and react on them
    checkTimeouts();

    // Sleep up to 1 millisecond if there is nothing to do
    waitForInterrupt();
    printSerialBusVoltage(500);
}

/**
 * Will be called by the Selfbus sblib main(), while no application is loaded.
 * In case we have a HAND_ACTUATION all LED's will blink at 2*BLINK_TIME (~1Hz) to indicate this state
 */
void loop_noapp(void)
{
#if defined(IO_TEST) && defined(HAND_ACTUATION)
    if (!bcu.programmingMode())
    {
        HandActuation::testIO(&handPins[0], NO_OF_HAND_PINS, BLINK_TIME);
    }
#endif
    waitForInterrupt();
    printSerialBusVoltage(500);
}

void ResetDefaultApplicationData()
{
#ifdef BUSFAIL
    AppData.relaysstate = 0x00;
#endif
}

#ifdef BUSFAIL
bool saveRelayState()
{
    AppData.relaysstate = getRelaysState();
    return AppNovSetting.StoreApplData((unsigned char*)&AppData, sizeof(ApplicationData));
}

void AppCallback::BusVoltageFail()
{
    pinMode(PIN_INFO, OUTPUT); // even in non DEBUG flash Info LED to display app data storing
    digitalWrite(PIN_INFO, 1);

    // write application settings to flash
    digitalWrite(PIN_INFO, !saveRelayState());
    stopApplication();

#ifdef DEBUG
    digitalWrite(PIN_RUN, 0); // switch RUN-LED off, to save some power
#endif
}

void AppCallback::BusVoltageReturn()
{
#ifdef DEBUG
    digitalWrite(PIN_RUN, 1); // switch RUN-LED ON
#endif
    //restore application settings
    if (!recallAppData()) // load custom application settings
    {
        // load default values
        ResetDefaultApplicationData();
    }
    bcu.begin(MANUFACTURER, DEVICETYPE, APPVERSION);
    initApplication(AppData.relaysstate);
}

int AppCallback::convertADmV(int valueAD)
{
    // good approximation between 10 & 30V for the 4TE-ARM controller
    if (valueAD > 2150)
        return 30000;
    else if (valueAD < 872)
        return 0;
    else
    {
        float valueADSquared = sq(valueAD);
        return      0.00000857674926702488f*valueADSquared*valueAD + // a*x^3
                   -0.0310784307851376f*valueADSquared +             // b*x^2
                   47.7234335386816f*valueAD +                       // c*x
               -14253.9303808124f;                                   // d
    }
    /*
     *  4TE ARM-Controller coefficients found with following measurements:
     *  ---------------------
     *  | Bus mV  ADC-Value |
     *  ---------------------
     *  |  9542    872      |
     *  | 10043    920      |
     *  | 11047   1017      |
     *  | 12008   1108      |
     *  | 13013   1202      |
     *  | 14012   1293      |
     *  | 15044   1382      |
     *  | 16024   1465      |
     *  | 17024   1545      |
     *  | 18012   1615      |
     *  | 19031   1686      |
     *  | 20044   1749      |
     *  | 20959   1801      |
     *  | 22003   1854      |
     *  | 23001   1901      |
     *  | 23987   1945      |
     *  | 24979   1985      |
     *  | 25999   2022      |
     *  | 26978   2055      |
     *  | 28006   2087      |
     *  | 29009   2115      |
     *  | 29873   2140      |
     *  | 30184   2150      |
     *  ---------------------
     *
    */
}

int AppCallback::convertmVAD(int valuemV)
{
    // good approximation between 10 & 30V for the 4TE-ARM controller
    if (valuemV >= 30184)
        return 2150;
    else if (valuemV < 9542)
        return 0;
    else

    return   -0.00000214162532145905f*sq(valuemV) + // a*x^2
              0.146795202310839f*valuemV +          // b*x
           -339.582791686125f;                      // c
    /*
     *  4TE ARM-Controller coefficients found with following measurements:
     *  ---------------------
     *  | Bus mV  ADC-Value |
     *  ---------------------
     *  |  9542    872      |
     *  | 10043    920      |
     *  | 11047   1017      |
     *  | 12008   1108      |
     *  | 13013   1202      |
     *  | 14012   1293      |
     *  | 15044   1382      |
     *  | 16024   1465      |
     *  | 17024   1545      |
     *  | 18012   1615      |
     *  | 19031   1686      |
     *  | 20044   1749      |
     *  | 20959   1801      |
     *  | 22003   1854      |
     *  | 23001   1901      |
     *  | 23987   1945      |
     *  | 24979   1985      |
     *  | 25999   2022      |
     *  | 26978   2055      |
     *  | 28006   2087      |
     *  | 29009   2115      |
     *  | 29873   2140      |
     *  | 30184   2150      |
     *  ---------------------
     *
    */
}

void AppUsrCallback::Notify(UsrCallbackType type)
{
    switch (type)
    {
        case UsrCallbackType::reset : // Reset after an ETS-application download or simple @ref APCI_BASIC_RESTART_PDU
            saveRelayState();
            break;

        default :
            break;
    }
}

#endif /* BUSFAIL */
