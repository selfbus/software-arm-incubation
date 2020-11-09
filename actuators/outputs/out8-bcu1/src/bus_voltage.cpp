/*
 *  bus_voltage.cpp
 *
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "bus_voltage.h"

#include <sblib/interrupt.h>
#include <sblib/io_pin_names.h>
#include <sblib/digital_pin.h>
#include <sblib/analog_pin.h>


// copy&paste from sblib/analog_pin.cpp
#define ADC_DONE  0x80000000      // ADC conversion complete
#define ADC_OVERRUN  0x40000000   // ADC overrun
#define ADC_START_NOW  (1 << 24)  // Start ADC now

#define ADC_RESOLUTION 1023 //10bit
#define ADC_MAX_VOLTAGE 3.3 // 3.3V VDD on LPC11xx

static volatile bool waitingADC_ = false; // true while we wait for an ADC-conversion to complete
static volatile bool busVoltageFailed_ = false; // true while bus voltage is to low
static volatile int threshold_ = 600; // 1.94V on 10bit resolution of the ADC
static volatile int ADChannel_ = AD7; // default AD channel for 4TE & TS_ARM controller

/*
 * BusVoltageFail() should be overwritten in the apps main
 */
void BusVoltageFail()
{
    waitForInterrupt();
};

/*
 * BusVoltageReturn() should be overwritten in the apps main
 */
void BusVoltageReturn()
{
    waitForInterrupt();
};

/*
 * ISR routine to handle ADC interrupt (parts are copy&paste from sblib/analog_pin.cpp)
 */
extern "C" void ADC_IRQHandler(void)
{
    // Disable interrupt so it won't keep firing
    disableInterrupt(ADC_IRQn);
    unsigned int regVal;
    bool busVoltageState;

    regVal = LPC_ADC->DR[ADChannel_];

    if (regVal & ADC_DONE)
    {
        LPC_ADC->CR &= 0xf8ffffff;  // Stop ADC
    }
    // This bit is 1 if the result of one or more conversions was lost and
    // overwritten before the conversion that produced the result.
    if (!(regVal & ADC_OVERRUN))
    {
        waitingADC_ = false;
        // this should be our ADC read out value
        regVal = (regVal >> 6) & 0x3ff;

        busVoltageState = regVal < threshold_;
        if (busVoltageState != busVoltageFailed_) // bus voltage state changed since last ADC-conversion
        {
            if (busVoltageState)
                BusVoltageFail();
            else
                BusVoltageReturn();
        }
        busVoltageFailed_ = busVoltageState; // preserve actual bus voltage state
    }
}

/*
 * constructor class BusVoltage (use BusVoltage::enableBusVRefMonitoring to configure AD-Pin & AD-Port...)
 */
BusVoltage::BusVoltage()
{
    ADPin_ = PIN_VBUS;
}


/*
 * enable bus voltage monitoring using AD-conversion
 */
void BusVoltage::enableBusVRefMonitoring(int ADPin, int ADChannel, float thresholdVoltage)
{
    ADPin_ = ADPin;
    ADChannel_ = ADChannel;
    threshold_ = (thresholdVoltage * ADC_RESOLUTION) / ADC_MAX_VOLTAGE; // convert voltage to ADC-resolution
    analogBegin();
    pinMode(ADPin_, INPUT_ANALOG); // for TS_ARM & 4TE controller Analog channel 7 (pin PIO1_11)
}

/*
 * disable bus voltage monitoring
 */
void BusVoltage::disableBusVRefMonitoring()
{
    clearPendingInterrupt(ADC_IRQn);
    disableInterrupt(ADC_IRQn);
    waitingADC_ = false;
    analogEnd();
}

/*
 * enables ADC ISR
 */
void BusVoltage::enableADCIsr(void)
{
    clearPendingInterrupt(ADC_IRQn);
    enableInterrupt(ADC_IRQn);
}

/*
 * start ADC conversion
 */
void BusVoltage::startBusVoltageRead()
{
    if (!pendingRead())
    {
        waitingADC_ = true;
        enableADCIsr();
        LPC_ADC->CR &= 0xffffff00;                      // CLKDIV = 0x13, CLKS=0x0 =>10bit resolution
        LPC_ADC->DR[ADChannel_];                           // read the channel to clear the "done" flag
        LPC_ADC->CR |= (1 << ADChannel_) | ADC_START_NOW;  // start the ADC reading
        LPC_ADC->INTEN = (1 << ADChannel_);                // enable only interrupt for channel
    }
}

bool BusVoltage::pendingRead()
{
    return waitingADC_;
}

void BusVoltage::checkPeriodic()
{
    startBusVoltageRead();
}

bool BusVoltage::failed()
{
    return busVoltageFailed_;
}

BusVoltage vBus = BusVoltage();


