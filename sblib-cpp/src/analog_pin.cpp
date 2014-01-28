/*
 *  analog_pin.cpp - Functions for analog I/O
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/analog_pin.h>

#include <sblib/platform.h>

// ADC conversion complete
#define ADC_DONE  0x80000000

// ADC overrun
#define ADC_OVERRUN  0x40000000

// Start ADC now
#define ADC_START_NOW  (1 << 24)

// Clock for AD conversion
#define ADC_CLOCK  2400000


void analogBegin()
{
    // Disable power down bit to the ADC block.
    LPC_SYSCON->PDRUNCFG &= ~(1<<4);

    // Enable AHB clock to the ADC.
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<13);

    LPC_ADC->CR = ((SystemCoreClock / LPC_SYSCON->SYSAHBCLKDIV) / ADC_CLOCK - 1) << 8;
}

void analogEnd()
{
    // Enable power down bit to the ADC block.
    LPC_SYSCON->PDRUNCFG |= 1<<4;

    // Disable AHB clock to the ADC.
    LPC_SYSCON->SYSAHBCLKCTRL &= ~(1<<13);
}

int analogRead(int channel)
{
    LPC_ADC->CR &= 0xffffff00;
    LPC_ADC->DR[channel]; // read the channel to clear the "done" flag

    LPC_ADC->CR |= (1 << channel) | ADC_START_NOW; // start the ADC reading

    unsigned int regVal;
    do
    {
        regVal = LPC_ADC->DR[channel];
    }
    while (!(regVal & ADC_DONE));

    LPC_ADC->CR &= 0xf8ffffff;  // Stop ADC

    // This bit is 1 if the result of one or more conversions was lost and
    // overwritten before the conversion that produced the result.
    if (regVal & ADC_OVERRUN)
        return 0;

    return (regVal >> 6) & 0x3ff;
}
