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

// The interrupt handler for the ADC busVoltageMonitor object
ADC_INTERRUPT_HANDLER(ADC_IRQHandler, busVoltageMonitor);

/*
 * ISR routine to handle ADC interrupt (parts are copy&paste from sblib/analog_pin.cpp)
 */
void BusVoltage::adcInterruptHandler()
{
    // dont remove, otherwise Isr will trigger all the time
    LPC_ADC->CR = _ADCR | (1 << _ADChannel); // start by rising edge of the timer

    unsigned int regVal = LPC_ADC->DR[_ADChannel];

    if (!(regVal & ADC_DONE))
        return;

    if (regVal & ADC_OVERRUN)
        return;

#ifdef DEBUG
    digitalWrite(PIN_INFO, 1);
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
        if ((isrBusVoltageState != BusVoltage::FAILED) && (_busVoltageFailTimeMs <= timelapsed))
        {
            isrBusVoltageState = BusVoltage::FAILED;
            if (_callback != nullptr)
            {
                _callback->BusVoltageFail();
            }
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
        if ((isrBusVoltageState != BusVoltage::OK) && (_busVoltageReturnTimeMs <= timelapsed))
        {
            if (isrBusVoltageState != BusVoltage::UNKNOWN)
            {
                isrBusVoltageState = BusVoltage::OK;
                if (_callback != nullptr)
                {
                    _callback->BusVoltageReturn();
                }
            }
            isrBusVoltageState = BusVoltage::OK;
        }
    }
#ifdef DEBUG
    digitalWrite(PIN_INFO, 0);
#endif
}

BusVoltage::BusVoltage()
                : _ADPin(PIN_VBUS)
                , _ADChannel(AD7)
                , _ADSampleFrequency(10000)
                , _thresholdVoltageFailed(20000)
                , _thresholdVoltageReturn(21000)
                , _busVoltageFailTimeMs(20)
                , _busVoltageReturnTimeMs(1500)
                , _callback(nullptr)
                , _timerMatchChannel(0)
                , isrbusVoltagethresholdFailed(1903)
                , isrbusVoltagethresholdReturn(1996)
                , isrmeanbusVoltage(0)
                , isrADSampleCount(0)
                , isrFailedFirstSysTick(0)
                , isrReturnedFirstSysTick(0)
                , isrBusVoltageState(BusVoltage::UNKNOWN)
{
}

void BusVoltage::adcTimerSetup(void)
{
    // mostly from out-cs-bim112
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
    isrFailedFirstSysTick = 0;
    isrReturnedFirstSysTick = 0;
    isrBusVoltageState = BusVoltage::UNKNOWN;
    NVIC_SetPriority(ADC_IRQn, 3); // set lowest priority for AD interrupt
    enableInterrupt(ADC_IRQn);
}

/**
 * setup Bus Voltage Monitoring.
 *
 * @param ADPin IO-Pin used for bus voltage monitoring.
 * @param ADChannel AD-Channel correspondending to the IO-Pin
 * @param ADSampleFrequency AD-conversion frequency in Hz
 * @param thresholdVoltageFailed Threshold for bus failing in milli-voltage
 * @param thresholdVoltageReturn Threshold for bus returning in milli-voltage
 * @param busVoltageFailTimeMs time (milliseconds) bus voltage (milli-voltage) must stay below, to trigger bus failing notification
 * @param busVoltageReturnTimeMs time (milliseconds) bus voltage (milli-voltage) must stay above, to trigger bus returning notification
 * @param *adTimer timer used for AD-conversion. Can be timer16_0 or timer32_0 from sblib/timer.h
 * @param timerMatchChannel timer match channel used for AD-conversion. Can be 0 or 1
 * @param callback instance of a sub-class of class BusVoltageCallback
 *
 * @return non-zero if successful.
 */
unsigned int BusVoltage::setup(unsigned int ADPin, unsigned int ADChannel, unsigned int ADSampleFrequency,
                               unsigned int thresholdVoltageFailed, unsigned int thresholdVoltageReturn,
                               unsigned int busVoltageFailTimeMs, unsigned int busVoltageReturnTimeMs,
                               Timer *adTimer, unsigned int timerMatchChannel, BusVoltageCallback *callback)
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
    _callback = callback;

    if (_callback != nullptr)
    {
        isrbusVoltagethresholdFailed = _callback->convertmVAD(_thresholdVoltageFailed);
        isrbusVoltagethresholdReturn = _callback->convertmVAD(_thresholdVoltageReturn);
    }
    else
    {
        isrbusVoltagethresholdFailed = _thresholdVoltageFailed;
        isrbusVoltagethresholdReturn = _thresholdVoltageReturn;
    }

    return 1;
}

/*
 * enable bus voltage monitoring using AD-conversion
 */
void BusVoltage::enable()
{
    adcTimerSetup();
    analogSetup();
    isrSetup();
}

/*
 * disable bus voltage monitoring
 */
void BusVoltage::disable()
{
    clearPendingInterrupt(ADC_IRQn);
    disableInterrupt(ADC_IRQn);

    // Enable power down bit to the ADC block.
    LPC_SYSCON->PDRUNCFG |= 1<<4;
    // Disable AHB clock to the ADC.
    LPC_SYSCON->SYSAHBCLKCTRL &= ~(1<<13);

    _adTimer->stop();
}

/*
 *  returns measured bus voltage in mV, returns -1 in case of invalid measurement
 */
int BusVoltage::valueBusVoltagemV()
{
    if (isrADSampleCount < 1)
        return -1;
    else if (_callback != nullptr)
    {
        return _callback->convertADmV(isrmeanbusVoltage);
    }
    else
        return isrmeanbusVoltage;
}

int BusVoltage::valueBusVoltageAD()
{
    return isrmeanbusVoltage;
}

bool BusVoltage::busFailed()
{
    return (isrBusVoltageState != BusVoltage::OK);
}

BusVoltage busVoltageMonitor = BusVoltage();
