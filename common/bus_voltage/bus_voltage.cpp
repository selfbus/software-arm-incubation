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

// from sblib/analog_pin.cpp
#define ADC_DONE 0x80000000      // ADC conversion complete
#define ADC_OVERRUN 0x40000000   // ADC overrun
#define ADC_CLOCK 2400000        // Clock for AD conversion

#define ADC_RESOLUTION 1023  //10bit
#define ADC_MAX_VOLTAGE 3405 // 3.405V VDD on LPC11xx
#define ADC_AD_SAMPLE_COUNT_MIN 100

// careful, variables can be changed any time by the Isr ADC_IRQHandler
static volatile bool isrCallBusFailed = false;             // true then the isr detected a bus fail
static volatile bool isrCallBusReturn = false;             // true then the isr detected a bus return
static volatile int isrbusVoltagethresholdFailed = 1903;   // AD-value threshold below which a bus failure should be reported
static volatile int isrbusVoltagethresholdReturn = 1996;   // AD-value threshold above which a bus return should be reported
static volatile int isrmeanbusVoltage = 0;                 // mean bus voltage as AD-value
static volatile unsigned int isrADSampleCount = 0;
static volatile unsigned int isrFailedFirstSysTick = 0;
static volatile unsigned int isrReturnedFirstSysTick = 0;
static volatile BusVoltage::State isrBusVoltageState = BusVoltage::UNKNOWN;

/*
 * ISR routine to handle ADC interrupt (parts are copy&paste from sblib/analog_pin.cpp)
 */
extern "C" void ADC_IRQHandler(void)
{
    // dont remove, otherwise Isr will trigger all the time
    LPC_ADC->CR = busVoltageMonitor._ADCR | (1 << busVoltageMonitor.getADChannel()); // start by rising edge of the timer

    unsigned int regVal = LPC_ADC->DR[busVoltageMonitor.getADChannel()];

    if (!(regVal & ADC_DONE))
        return;

    if (regVal & ADC_OVERRUN)
        return;

#ifdef DEBUG
    digitalWrite(PIN_INFO, !digitalRead(PIN_INFO));
#endif

    regVal = (regVal >> 6) & 0x3ff;                       // this is the ADC read out value
    regVal = (regVal * ADC_MAX_VOLTAGE) / ADC_RESOLUTION; // convert AD-value to mV

    isrADSampleCount++;
    if (isrADSampleCount > 1)
        isrmeanbusVoltage = (isrmeanbusVoltage + regVal) / 2; // calculate mean bus voltage
    else
        isrmeanbusVoltage = regVal;

    if (isrADSampleCount < ADC_AD_SAMPLE_COUNT_MIN)
        return;

    if (isrmeanbusVoltage <= isrbusVoltagethresholdFailed)  // bus voltage to low
    {
        isrReturnedFirstSysTick = 0;
        unsigned int SysTickOverflow = (millis() < isrFailedFirstSysTick);
        if ((isrFailedFirstSysTick == 0) || (SysTickOverflow != 0))
        {
            digitalWrite(PIN_INFO, 1);
            isrFailedFirstSysTick = millis();
            return;
        }

        unsigned int timelapsed = millis() - isrFailedFirstSysTick;
        if ((isrBusVoltageState != BusVoltage::FAILED) && (busVoltageMonitor.getBusVoltageFailTimeMs() <= timelapsed))
        {
            isrCallBusFailed = true;
            isrBusVoltageState = BusVoltage::FAILED;
        }
    }
    else if ((isrmeanbusVoltage > isrbusVoltagethresholdReturn)) // bus voltage returning
    {
        isrFailedFirstSysTick = 0;
        unsigned int SysTickOverflow = (millis() < isrReturnedFirstSysTick);
        if ((isrReturnedFirstSysTick == 0) || (SysTickOverflow != 0))
        {
            digitalWrite(PIN_INFO, 0);
            isrReturnedFirstSysTick = millis();
            return;
        }

        unsigned int timelapsed = millis() - isrReturnedFirstSysTick;
        if ((isrBusVoltageState != BusVoltage::OK) && (busVoltageMonitor.getBusVoltageReturnTimeMs() <= timelapsed))
        {
            if (isrBusVoltageState != BusVoltage::UNKNOWN)
            {
                isrCallBusReturn = true;
            }
            isrBusVoltageState = BusVoltage::OK;
        }
    }
#ifdef DEBUG
    digitalWrite(PIN_INFO, !digitalRead(PIN_INFO));
#endif
}

void BusVoltage::adcTimerSetup(void)
{
    // Der Timer ist die eigentliche Zeitbasis der Sampling-Routinen, er startet periodisch den ADC.
    // Die Interrupt-Routine auf ADC-Conversion Done macht dann alles weitere.
    // Es werden die Timer-Funktionen aus Timer.h/.cpp genutzt
    _adTimer->begin();
    _adTimer->noInterrupts();
    _adTimer->prescaler(0);
    _adTimer->matchMode(_timerMatchChannel, TOGGLE | RESET); // Toggle und Reset mit Ch 0 (Der Timer hat 4 "Channels" je Timer)
    // ==> Dadurch, dass nur "getoggled" werden kann, muss die halbe Matchperiode konfiguriert werden!
    _adTimer->match(_timerMatchChannel, (SystemCoreClock / LPC_SYSCON->SYSAHBCLKDIV) / _ADSampleFrequency / 2 - 1);
    _adTimer->start();
}
void BusVoltage::analogSetup()
{
    unsigned int conversionStartBits = 0;

    // conversion possible with CT16B0_MAT0 or CT16B0_MAT1 (timer16_0)
    // or                       CT32B0_MAT0 or CT32B0_MAT1 (timer32_0)
    if (_adTimer == &timer16_0)
    {
        if (_timerMatchChannel == 0)
            conversionStartBits = 0x06000000;
        else
            conversionStartBits = 0x07000000;
    }
    else if (_adTimer == &timer32_0)
    {
        if (_timerMatchChannel == 0)
            conversionStartBits = 0x04000000;
        else
            conversionStartBits = 0x05000000;
    }

    pinMode(_ADPin, INPUT_ANALOG);              // for TS_ARM & 4TE controller Analog channel 7 (pin PIO1_11)
    LPC_SYSCON->PDRUNCFG &= ~(1<<4);            // Disable power down bit to the ADC block.
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<13);       // Enable AHB clock to the ADC.
    // Vorteiler des ADC (SystemCoreClock momentan 48MHz, der Teiler SYSAHBCLKDIV wirkt sich auf Core und Peripherie aus
    LPC_ADC->CR = ((SystemCoreClock / LPC_SYSCON->SYSAHBCLKDIV) / ADC_CLOCK - 1) << 8;
    LPC_ADC->CR |= conversionStartBits;         // Wandlungsstart über CT16B0_MAT1
    LPC_ADC->GDR;                               // Dummy Lesezugriff auf das Global data register
    LPC_ADC->DR[_ADChannel];                    // read the channel to clear the "done" flag
    _ADCR = LPC_ADC->CR;                        // save content of register CR for later use in the adcIsr
    LPC_ADC->CR |= (1 << _ADChannel);           // start ADC reading
}

void BusVoltage::isrSetup()
{
    isrADSampleCount = 0;
    isrmeanbusVoltage = 0;
    isrCallBusFailed = false;
    isrCallBusReturn = false;
    isrFailedFirstSysTick = 0;
    isrReturnedFirstSysTick = 0;
    isrBusVoltageState = BusVoltage::UNKNOWN;
    NVIC_SetPriority(ADC_IRQn, 3); // set lowest priority for AD interrupt
    enableInterrupt(ADC_IRQn);
}


/*
 * setup bus voltage monitoring using AD-conversion
 */
unsigned int BusVoltage::setupMonitoring(unsigned int ADPin, unsigned int ADChannel, unsigned int ADSampleFrequency,
                                         unsigned int thresholdVoltageFailed, unsigned int thresholdVoltageReturn,
                                         unsigned int busVoltageFailTimeMs, unsigned int busVoltageReturnTimeMs,
                                         Timer *adTimer, unsigned int timerMatchChannel)
{
    if ((adTimer != &timer16_0) && (adTimer != &timer32_0))
        return 0;
    if ((timerMatchChannel != 0) && (timerMatchChannel != 1))
        return 0;

    _ADPin = ADPin;
    _ADChannel = ADChannel;
    _ADSampleFrequency = ADSampleFrequency;
    _thresholdVoltageFailed = thresholdVoltageFailed;
    _thresholdVoltageReturn = thresholdVoltageReturn;
    _busVoltageFailTimeMs = busVoltageFailTimeMs;
    _busVoltageReturnTimeMs = busVoltageReturnTimeMs;
    _adTimer = adTimer;
    _timerMatchChannel = timerMatchChannel;
    isrbusVoltagethresholdFailed = convertmVAD(_thresholdVoltageFailed);
    isrbusVoltagethresholdReturn = convertmVAD(_thresholdVoltageReturn);

    return 1;
}

/*
 * enable bus voltage monitoring using AD-conversion
 */
void BusVoltage::enableMonitoring()
{
    adcTimerSetup();
    analogSetup();
    isrSetup();
}

/*
 * disable bus voltage monitoring
 */
void BusVoltage::disableMonitoring()
{
    clearPendingInterrupt(ADC_IRQn);
    disableInterrupt(ADC_IRQn);

    // Enable power down bit to the ADC block.
    LPC_SYSCON->PDRUNCFG |= 1<<4;
    // Disable AHB clock to the ADC.
    LPC_SYSCON->SYSAHBCLKCTRL &= ~(1<<13);

    _adTimer->stop();
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
    return (isrBusVoltageState == BusVoltage::FAILED);
}

BusVoltage busVoltageMonitor = BusVoltage();


