/*
 * RelSpi.h - Relay SPI functions
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2017 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef RELSPI_H_
#define RELSPI_H_

#include <config.h>

class RelSpi;

extern RelSpi relspi;

class RelSpi
{
public:

 RelSpi(void);

 int ReadRx(void);
 void GetRxData(unsigned* Data, unsigned size);
 void SetTxData(unsigned* Data, unsigned size);
 void StartTransfer(void);

protected:
 int TxPtr;
 int RxPtr;
 int RxSize;
 unsigned ChainDataTx[SPICHAINLEN+1];
 unsigned ChainDataRx[SPICHAINLEN+1];
};


#endif /* RELSPI_H_ */
