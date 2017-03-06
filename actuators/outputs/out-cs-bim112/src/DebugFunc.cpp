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
  calc = sqrt((float)IsrData.RegSqr[ChIdx] / (float)BUFSIZE);
  if (ChIdx & 1)
  {
   // LowRange
   curr = (int)(calc*MAXCURRLOWRANGE/512*10000);
  } else {
   // High Range
   curr = (int)(calc*MAXCURRHIGHRANGE/512*10000);
  }
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

