/*
 *  debug.cpp - A class that provides print() and println() over the
 *              connected hardware debugger.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/debug.h>

#include <sblib/platform.h>


// This feature does not work in LPCXpresso on the LPC111x yet
//#define DEBUG_ACTIVE  (CoreDebug->DHCSR&CoreDebug_DHCSR_C_DEBUGEN_Msk)
#define DEBUG_ACTIVE  1

#if CONFIG_DRIVER_PRINTF_REDLIBV2==1
  extern int __sys_write(int zero, const char *s, int len);
  extern int __sys_readc();
#else
  extern int __write(int zero, const char *s, int len);
//  extern int __read(int zero, char *s, int len);
//  extern int __readc();
#endif


int Debug::write(byte ch)
{
    return write((const byte*) &ch, 1);
}

int Debug::write(const byte* data, int count)
{
#ifndef IAP_EMULATION
    if (DEBUG_ACTIVE)
        return __write(0, (const char*) data, count);
#endif
    return 0;
}
