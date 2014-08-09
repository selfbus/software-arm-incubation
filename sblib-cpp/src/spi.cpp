/*
 *  spi.cpp - Serial peripheral interface (SPI).
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/spi.h>
#include <sblib/timer.h>


// SPI clock-out phase control bit
#define SSP_CR0_CPHA_FIRSTCLOCK  0
#define SSP_CR0_CPHA_SECONDCLOCK (1<<7)

// SPI clock-out polarity
#define SSP_CR0_CPOL_LOW     0
#define SSP_CR0_CPOL_HIGH    (1<<6)

// SPI controller enabled
#define SSP_CR1_ENABLED      (1<<1)

// SPI master/slave mode
#define SSP_CR1_MASTER       0
#define SSP_CR1_SLAVE        (1<<2)

/*
 * Macros for the SR register
 */
// SPI status TX FIFO Empty bit
#define SSP_SR_TFE      ((uint32_t)(1<<0))
// SPI status TX FIFO not full bit
#define SSP_SR_TNF      ((uint32_t)(1<<1))
// SPI status RX FIFO not empty bit
#define SSP_SR_RNE      ((uint32_t)(1<<2))
// SPI status RX FIFO full bit
#define SSP_SR_RFF      ((uint32_t)(1<<3))
// SPI status SPI Busy bit
#define SSP_SR_BSY      ((uint32_t)(1<<4))
// SPI SR bit mask
#define SSP_SR_BITMASK  ((uint32_t)(0x1F))

// SPI data bit mask
#define SSP_DR_BITMASK(n) ((n) & 0xFFFF)

// ICR bit mask
#define SSP_ICR_BITMASK  0x03


// The SPI port registers
static LPC_SSP_TypeDef* const ports[2] = { LPC_SSP0, LPC_SSP1 };


SPI::SPI(int portNum)
:port(*ports[portNum])
,clockDiv(100)
{
    /* Enable AHB clock to the GPIO domain. */
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

    if (portNum == 0) // SPI port 0
    {
        // Reset SSP0 peripheral
        LPC_SYSCON->PRESETCTRL |= 1;

        // Enable the clock for the SPI port
        LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 11;

        // Set the clock divider
        LPC_SYSCON->SSP0CLKDIV = 2;
    }
    else // SPI port 1
    {
        // Reset SSP1 peripheral
        LPC_SYSCON->PRESETCTRL |= 2;

        // Enable the clock for the SPI port
        LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 18;

        // Set the clock divider
        LPC_SYSCON->SSP1CLKDIV = 2;
    }

    // Use 8 bit data size, SPI frame format, bus clock low between frames, no clock divider.
    port.CR0 = SSP_CR0_CPHA_SECONDCLOCK | SSP_CR0_CPOL_LOW | (SPI_DATA_8BIT - 1);

    // Use master mode, SPI disabled (calling begin() enables SPI)
    port.CR1 = SSP_CR1_MASTER;

    // Set the clock prescaler register
    port.CPSR = 2;
}

void SPI::setClockDivider(int div)
{
    port.CR0 = (port.CR0 & ~0xff00) | (((div - 1) & 255) << 8);
}

void SPI::setDataSize(SpiDataSize dataSize)
{
    port.CR0 = (port.CR0 & ~15) | ((dataSize - 1) & 15);
}

void SPI::begin()
{
    port.CR1 |= SSP_CR1_ENABLED;
}

void SPI::end()
{
    port.CR1 &= ~SSP_CR1_ENABLED;
}

int SPI::transfer(int val, SpiTransferMode transferMode)
{
    int tmpVal;

    // Clear all remaining data in the receive FIFO
    while (port.SR & SSP_SR_RNE)
        tmpVal = SSP_DR_BITMASK(port.DR);

    // Clear the interrupt status
    port.ICR = SSP_ICR_BITMASK;

    // Write value to the buffer
    LPC_SSP0->DR = val;

    // Wait for completed transfer
    int start = millis();
    while (port.SR & SSP_SR_BSY)
        ;
    int waited = elapsed(start);

    // Return read value
    return LPC_SSP0->DR;
}
