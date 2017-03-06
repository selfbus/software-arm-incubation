/*
 * RelSpi.cpp - Relay SPI functions
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2017 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/platform.h>
#include <RelSpi.h>
#include <Relay.h>
#include <AdcIsr.h>

#if SPICHAINLEN > 8
#error SPI chain length greater than hardware fifo!
#endif

RelSpi relspi;

RelSpi::RelSpi(void)
{
 TxPtr = 0;
 RxPtr = 0;
 RxSize = 0;
 LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

 // Disable reset of SSP0
 LPC_SYSCON->PRESETCTRL |= 1;

 // Enable the clock for the SPI port
 LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 11;

 // Set the clock divider to 2
 LPC_SYSCON->SSP0CLKDIV = 2;

 // Use 8 bit data size, SPI frame format, bus clock high between frames CPOL=1, CPHA=1, clock divider 15.
 SPISSP->CR0 = 0x0000ec7;

 // Set the clock prescaler register
 SPISSP->CPSR = 16;

 // Resulting SCK frequency 48MHz/2/16/15 = 100kHz
#ifdef SPICSEMULATION
 pinMode(PIOSPICS,  OUTPUT); // Der echte CS-Port ist beim Tasta-interface nicht rausgeführt, also manuelle Bedienung
 digitalWrite(PIOSPICS, true);
#else
 pinMode(PIOSPICS,  OUTPUT | SPI_SSEL); // Der echte CS-Port wird verwendet
#endif
 pinMode(PIOSPIMOSI,  OUTPUT | SPI_MOSI);
#ifdef PIOSPIMISO
 pinMode(PIOSPIMISO,  INPUT | SPI_MISO);
#endif
 pinMode(PIOSPISCK, OUTPUT | SPI_CLOCK);

 // Use master mode, enable SPI
 SPISSP->CR1 = 2;
}

int RelSpi::ReadRx(void)
{
 RxPtr = 0;
 RxSize = 0;
 while (SPISSP->SR & 0x10); // SPI ist Busy, das sollte hier nie vorkommen. ReadRx wohl zu schnell nach StartTransfer aufgerufen!
 while (SPISSP->SR & 4)
 {
  ChainDataRx[RxSize++] = SPISSP->DR;
  // Mehr Daten als da sein sollten?
  if (RxSize > SPICHAINLEN)
  {
   RxSize = -1;
   // Buffer leeren
   while (SPISSP->SR & 4)
   {
    SPISSP->DR;
   }
   break;
  }
 }
 return RxSize;
}

void RelSpi::GetRxData(unsigned* Data, unsigned size)
{
 while ((RxPtr < RxSize) && (size-- > 0))
 {
  *Data++ = ChainDataRx[RxPtr++];
 }
}

void RelSpi::SetTxData(unsigned* Data, unsigned size)
{
 while ((TxPtr < SPICHAINLEN) && (size-- > 0))
 {
  ChainDataTx[TxPtr++] = *Data++;
 }
}

void RelSpi::StartTransfer(void)
{
 while (SPISSP->SR & 0x10); // SPI ist Busy, das sollte hier nie vorkommen. StartTransfer wohl zu oft aufgerufen!
#ifdef SPICSEMULATION
 digitalWrite(PIOSPICS, false);
#else
 noInterrupts();
#endif
 for(unsigned i=0;i<SPICHAINLEN;i++)
 {
  SPISSP->DR = ChainDataTx[i];
 }
#ifdef SPICSEMULATION
 IsrData.SpiActive = true; // Die ADC-Isr wird missbraucht, das Chipselect wieder zurückzusetzen.
#else
 interrupts();
#endif
 TxPtr = 0;
}
