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

// SPI loopback mode
#define SSP_CR1_LOOPBACK     (1<<0)
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

// Convert SPI data size to bit offset
#define SPI_DATA_SIZE_OFFS(x) ((x - 1)  & 15)


// The SPI port registers
static LPC_SSP_TypeDef* const ports[2] = { LPC_SSP0, LPC_SSP1 };


SPI::SPI(int portNum, int mode)
:port(*ports[portNum])
,clockDiv(100)
{
    // Enable AHB clock to the GPIO domain.
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

    // Disable reset of the SSP peripheral
    LPC_SYSCON->PRESETCTRL |= portNum + 1;

    if (portNum == 0) // SPI port 0
    {
        // Enable the clock for the SPI port
        LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 11;

        // Set the clock divider
        LPC_SYSCON->SSP0CLKDIV = 2;
    }
    else // SPI port 1
    {
        // Enable the clock for the SPI port
        LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 18;

        // Set the clock divider
        LPC_SYSCON->SSP1CLKDIV = 2;
    }

    // Use 8 bit data size, SPI frame format, bus clock low between frames, no clock divider.
    port.CR0 = mode | SPI_DATA_SIZE_OFFS(SPI_DATA_8BIT);

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
    port.CR0 = (port.CR0 & ~15) | SPI_DATA_SIZE_OFFS(dataSize);
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
        tmpVal = port.DR;

    // Clear the interrupt status
    port.ICR = SSP_ICR_BITMASK;

    // Write value to the buffer
    LPC_SSP0->DR = val;

    // Wait for completed transfer
    while (port.SR & SSP_SR_BSY)
        ;

    // Return read value
    return LPC_SSP0->DR;
}

#ifdef SPI_BLOCK_TRANSFER

static SPI * instances[2];

void SPI::transferBlock(uint16_t * sndData, int bytes, uint16_t * recData, bool asynchron)
{
    int tmpVal;

    // Clear all remaining data in the receive FIFO
    while (port.SR & SSP_SR_RNE)
        tmpVal = port.DR;

    this->sndData = sndData;
    this->recData = recData;
    this->sndCount = this->recCount = bytes;
    this->errors   = 0;
    this->finished = false;
    // Clear the interrupt status
    port.ICR = SSP_ICR_BITMASK;

    continueBlockTransfer();
    if (! asynchron)
    {
        finalizeBlockTransfer();
    }
    else
    {
        int no = & port == LPC_SSP0 ? 0 : 1;
        instances[no] = this;
        port.IMSC    |= (1 << 3); // XXX
        if (! no)
        {
            NVIC_EnableIRQ(SSP0_IRQn);
        }
        else
        {
            NVIC_EnableIRQ(SSP1_IRQn);
        }
    }
}

void SPI::continueBlockTransfer(void)
{
    int tmpVal;
    int maxCount = 8;
    while (sndCount && (port.SR & SSP_SR_TNF) && maxCount)
    {
        sndCount--;
        maxCount--;
        if (port.SR & SSP_SR_RNE)
        {
            recCount--;
            tmpVal = port.DR;
            if (recData) *recData++ = tmpVal;
        }
        LPC_SSP0->DR = *sndData++;
        errors |= port.RIS;
       }
    while (recCount && (port.SR & SSP_SR_RNE))
    {
        recCount--;
        tmpVal = port.DR;
        if (recData) *recData++ = tmpVal;
        errors |= port.RIS;
    }
    if (!sndCount)
    {
        int no        = & port == LPC_SSP0 ? 0 : 1;
        if (! no)
        {
            NVIC_DisableIRQ(SSP0_IRQn);
        }
        else
        {
            NVIC_DisableIRQ(SSP1_IRQn);
        }
        instances[no] = 0;
        port.IMSC    &= ~(1 << 3); // XXX
        finished      = true;
    }
}

void SPI::finalizeBlockTransfer(void)
{
    while (recCount)
        continueBlockTransfer();
}

extern "C" {
volatile uint32_t int_ssp_0 = 0;

void SSP0_IRQHandler (void)
{
    int_ssp_0++;
    instances[0]->continueBlockTransfer();
}
void SSP1_IRQHandler (void)
{
    instances[1]->continueBlockTransfer();
}

}
#endif
