/*
 *  DebugFunc.cpp - Some debug functions
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
#include <config.h>
#include <AdcIsr.h>
#include <Relay.h>
#include <RelSpi.h>
#include <sblib/serial.h>
#include <DebugFunc.h>
#include <math.h>

void SerialPrintSetup(void)
{
 serial.begin(115200);
}

ALWAYS_INLINE float Square(float a)
{ // Mir ist nicht 100% klar, ob pow(a,2) nur optimiert a*a berechnet, oder doch eine universelle Exponentialfunktion. Daher hier:
 return a*a;
}

//#define DEBUGLSBOUTPUT
void SerialPrintCurrents(void)
{
 unsigned curr;
 float calc;
 for (unsigned ChIdx=0; ChIdx < (CHANNELCNT*2); ChIdx++)
 {
#ifdef DEBUGLSBOUTPUT
  calc = sqrt((float)IsrData.RegSqr[ChIdx] / (float)BUFSIZE) *100;
  curr = (int)calc;
#else
  if ((IsrData.RegOvr[ChIdx]) < OFSCOMPOVRLIM)
  {
   serial.print("0, ");
  } else {
   serial.print("1, ");
  }
  //calc = sqrt((float)IsrData.RegSqr[ChIdx] / (float)BUFSIZE);
  if (ChIdx & 1)
  {
   // LowRange
   //curr = (int)(calc*(float)IsrData.GainCorr[(ChIdx << 1)+1]*MAXCURRLOWRANGE*10000/512/32768);
   // Berechnung mit den gleichen Formeln wie in AdcIsrCurrFilt
   calc =
     (float)IsrData.RegSqr[ChIdx] *
     Square((float)IsrData.GainCorr[ChIdx]) *
     Square(MAXCURRLOWRANGE/512/32768);
  } else {
   // High Range
   //curr = (int)(calc*(float)IsrData.GainCorr[(ChIdx << 1)]*MAXCURRHIGHRANGE*10000/512/32768);
   calc =
     (float)IsrData.RegSqr[ChIdx] *
     Square((float)IsrData.GainCorr[ChIdx]) *
     Square(MAXCURRHIGHRANGE/512/32768);
  }
  curr = sqrt(calc / (float)BUFSIZE)*10000;
#endif
  serial.print(IsrData.RegOvr[ChIdx]);
  serial.print(", ");
  serial.print(curr);
  serial.print("*0,1mA, ");
 }
 for (unsigned ChIdx=0; ChIdx < (CHANNELCNT*2); ChIdx++)
 {
  serial.print(IsrData.Offs[ChIdx]);
  serial.print(", ");
 }
 serial.println("");
}

