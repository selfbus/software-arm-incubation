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

#define ADC_RESOLUTION 1023  //10bit
#define ADC_MAX_VOLTAGE 3300 // 3.3V VDD on LPC11xx
#define ADC_AD_SAMPLE_COUNT_MIN 10

// careful, variables can change any time by the Isr ADC_IRQHandler
static volatile bool isrCallBusFailed = false;             // true then the isr detected a bus fail
static volatile bool isrCallBusReturn = false;             // true then the isr detected a bus return
static volatile bool isrwaitingADC = false;                // true while we wait for an ADC-conversion to complete
static volatile bool isrbusVoltageFailed = false;          // true while bus voltage has failed
static volatile int isrbusVoltagethresholdFailed = 1903;   // AD-value threshold below which a bus failure should be reported
static volatile int isrbusVoltagethresholdReturn = 1996;   // AD-value threshold above which a bus return should be reported
static volatile int isrADChannel = AD7;                    // default AD channel for 4TE & TS_ARM controller
static volatile  int isrmeanbusVoltage = 0;                // mean bus voltage as AD-value
static volatile unsigned int isrADSampleCount = 0;

/*
 * ISR routine to handle ADC interrupt (parts are copy&paste from sblib/analog_pin.cpp)
 */
extern "C" void ADC_IRQHandler(void)
{
    // Disable interrupt so it won't keep firing
    disableInterrupt(ADC_IRQn);
    unsigned int regVal;

    regVal = LPC_ADC->DR[isrADChannel];

    if (regVal & ADC_DONE)
    {
        LPC_ADC->CR &= 0xf8ffffff;  // Stop ADC
    }

    // This bit is 1 if the result of one or more conversions was lost and
    // overwritten before the conversion that produced the result.
    if (regVal & ADC_OVERRUN)
        return;


    isrwaitingADC = false;
    // this should be our ADC read out value
    regVal = (regVal >> 6) & 0x3ff;

    // convert AD-value to mV
    regVal = (regVal * ADC_MAX_VOLTAGE) / ADC_RESOLUTION;

    isrADSampleCount++;
    if (isrADSampleCount > 1)
    {
        isrmeanbusVoltage = (isrmeanbusVoltage + regVal) / 2; // calculate mean bus voltage
    }
    else
    {
        isrmeanbusVoltage = regVal;
    }

    if (isrADSampleCount >= ADC_AD_SAMPLE_COUNT_MIN)
    {
        if (!isrbusVoltageFailed && (isrmeanbusVoltage <= isrbusVoltagethresholdFailed)) // bus voltage failed
        {
            isrbusVoltageFailed = true;
            isrCallBusFailed = true;
        }
        else if (isrbusVoltageFailed && (isrmeanbusVoltage > isrbusVoltagethresholdReturn)) // bus voltage returned
        {
            isrbusVoltageFailed = false;
            isrCallBusReturn = true;
        }
    }
}

/*
 * constructor class BusVoltage (use BusVoltage::enableBusVRefMonitoring to configure AD-Pin & AD-Port...)
 */
BusVoltage::BusVoltage()
{
    ADPin_ = PIN_VBUS;
    resetIsrData();
}


/*
 * resets AD data used in the Isr
 */
void BusVoltage::resetIsrData()
{
    isrwaitingADC = false;
    isrbusVoltageFailed = false;
    isrADSampleCount = 0;
    isrmeanbusVoltage = 0;
    isrCallBusFailed = false;
    isrCallBusReturn = false;
}

/*
 * enable bus voltage monitoring using AD-conversion
 */
void BusVoltage::enableBusVRefMonitoring(int ADPin, int ADChannel, unsigned int thresholdVoltageFailed, unsigned int thresholdVoltageReturn)
{
    ADPin_ = ADPin;
    isrADChannel = ADChannel;
    isrbusVoltagethresholdFailed = convertmVAD(thresholdVoltageFailed);
    isrbusVoltagethresholdReturn = convertmVAD(thresholdVoltageReturn);
    analogBegin();
    pinMode(ADPin_, INPUT_ANALOG); // for TS_ARM & 4TE controller Analog channel 7 (pin PIO1_11)
    resetIsrData();
}

/*
 * disable bus voltage monitoring
 */
void BusVoltage::disableBusVRefMonitoring()
{
    clearPendingInterrupt(ADC_IRQn);
    disableInterrupt(ADC_IRQn);
    isrwaitingADC = false;
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
        isrwaitingADC = true;
        enableADCIsr();
        LPC_ADC->CR &= 0xffffff00;                          // CLKDIV = 0x13, CLKS=0x0 =>10bit resolution
        LPC_ADC->DR[isrADChannel];                          // read the channel to clear the "done" flag
        LPC_ADC->CR |= (1 << isrADChannel) | ADC_START_NOW; // start the ADC reading
        LPC_ADC->INTEN = (1 << isrADChannel);               // enable only interrupt for channel
    }
}

bool BusVoltage::pendingRead()
{
    return isrwaitingADC;
}

void BusVoltage::checkPeriodic()
{
    if (isrCallBusFailed) // isr sets isrCallBusFailed to true when it detects a bus fail
    {
        isrCallBusFailed = false;
        BusVoltageFail();
    }
    else if (isrCallBusReturn) // isr sets isrCallBusReturn to true when it detects a bus fail
    {
        isrCallBusReturn = false;
        BusVoltageReturn();
    }
    else
         startBusVoltageRead();
}

/*
 *  returns measured bus voltage in mV, returns -1 in case of invalid measurement
 */
int BusVoltage::valuemV()
{
    if (isrADSampleCount < 1)
        return -1;
    else
        return convertADmV(isrmeanbusVoltage);
}

bool BusVoltage::failed()
{
    return isrbusVoltageFailed;
}

BusVoltage vBus = BusVoltage();


