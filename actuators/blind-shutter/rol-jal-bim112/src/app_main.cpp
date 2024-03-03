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
#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>
#include "app_main.h"
#include "app-rol-jal.h"

#ifdef BUSFAIL
#    include <sblib/math.h>
#    include "bus_voltage.h"
#    include "app_nov_settings.h"
#endif

#ifdef BUSFAIL
    NonVolatileSetting AppNovSetting(0xEA00, 0x100);  // flash-storage for application relevant parameters
    ApplicationData AppData;
    AppCallback callback;
#endif

APP_VERSION("SBrol   ", "1", "11")

// Hardware version. Must match the product_serial_number in the VD's table hw_product
const HardwareVersion hardwareVersion[] =
{
    ///\todo implement missing 1, 2, 8- fold versions
    {4, 0x4578, { 0, 0, 0, 0, 0x0, 0x29 }} // JAL-0410.01 Shutter Actuator 4-fold, 4TE, 230VAC, 10A
    // {8, 0x46B8, { 0, 0, 0, 0, 0x0, 0x28 }}  // JAL-0810.01 Shutter Actuator 8-fold, 8TE, 230VAC,10A
};

const HardwareVersion * currentVersion;

Timeout timeout;

void initSerial()
{
#ifdef DEBUG_SERIAL
    serial.setRxPin(PIO2_7);
    serial.setTxPin(PIO2_8);
    serial.begin(115200);
    serial.println("rol-jal-bim112 serial debug started");
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
 * This function is called by the Selfbus's library main when the processor is started or reset.
 */
BcuBase* setup()
{
    ///\todo read some ID pins to determine which version is attached

    currentVersion = & hardwareVersion[0];
    bcu.begin(MANUFACTURER, currentVersion->hardwareVersion[5], APPVERSION);  // we are a MDT shutter/blind actuator, version 2.8
    bcu.setHardwareType(currentVersion->hardwareVersion, sizeof(currentVersion->hardwareVersion));

    pinMode(PIN_INFO, OUTPUT); // Info LED
    pinMode(PIN_RUN,  OUTPUT); // Run LED

    // Running the controller in a closed housing makes these LEDs useless - they just consume power
    // additionally at the moment the rol-jal application does not make use of these LEDs
    // check config file to toggle the use
#ifndef USE_DEV_LEDS
    digitalWrite(PIN_INFO, 0);
    digitalWrite(PIN_RUN, 0);
#endif

    // enable bus voltage monitoring
    startBusVoltageMonitoring();

    recallAppData();

#ifdef BUSFAIL
    initApplication(AppData.channelPositions, AppData.channelSlatPositions);
#else
    initApplication();
#endif

    timeout.start(1);
    return (&bcu);
}

/**
 * The main processing loop while a KNX-application is loaded
 */
void loop()
{
    int objno;
    // Handle updated communication objects
    while ((objno = bcu.comObjects->nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }
    // check the periodic functions of the application
    checkPeriodicFuntions();

    // Sleep up to 1 millisecond if there is nothing to do
    waitForInterrupt();
}

/**
 * The processing loop while no KNX-application is loaded
 */
void loop_noapp()
{
    // no application is loaded, make sure that all relays are off
    pinMode(PIN_PWM, OUTPUT);  // configure PWM Pin as output when application is not loaded
    digitalWrite(PIN_PWM, 1);  // set PWM Pin to high so all relays will be off

    if ((handAct != nullptr) && (!bcu.programmingMode()))
    {
        HandActuation::testIO(&handPins[0], NO_OF_HAND_PINS, BLINK_TIME);
    }
}


void ResetDefaultApplicationData()
{
#ifdef BUSFAIL
    for(int i = 0; i < NO_OF_CHANNELS; i++){
        AppData.channelPositions[i] = 0;
        AppData.channelSlatPositions[i] = 0;
    }
#endif
}

#ifdef BUSFAIL
void AppCallback::BusVoltageFail()
{
    pinMode(PIN_INFO, OUTPUT); // even in non DEBUG flash Info LED to display app data storing
    digitalWrite(PIN_INFO, 1);

     getChannelPositions(AppData.channelPositions, AppData.channelSlatPositions);
    // write application settings to flash
    if (AppNovSetting.StoreApplData((unsigned char*)&AppData, sizeof(ApplicationData)))
        digitalWrite(PIN_INFO, 0);
    else
        digitalWrite(PIN_INFO, 1);
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
    bcu.begin(MANUFACTURER, currentVersion->hardwareVersion[5], APPVERSION);
    initApplication(AppData.channelPositions, AppData.channelSlatPositions);
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
#endif /* BUSFAIL */

