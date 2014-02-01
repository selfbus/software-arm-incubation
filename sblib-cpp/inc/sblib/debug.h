/*
 *  debug.h - A class that provides print() and println() over the
 *            connected hardware debugger.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_debug_h
#define sblib_debug_h

#include <sblib/print.h>

/**
 * A class for printing to the connected debugger on the PC. Please note that
 * this form of debug output is very slow as it stops the ARM each time something
 * is printed.
 *
 * Uses semihosting to send the print output to a window in the debugger
 * on the PC.
 *
 * Example:
 * <p>
 *       Debug debug;
 *       debug.println("Hello world!");
 * </p>
 */
class Debug: public Print
{
    /**
     * Write a number of bytes.
     *
     * @param data - the bytes to write.
     * @param count - the number of bytes to write.
     *
     * @return The number of bytes that were written.
     */
    virtual int write(const byte* data, int count);

    /**
     * Write a single byte.
     *
     * @param ch - the byte to write.
     *
     * @return 1 if the byte was written, 0 if not.
     */
    virtual int write(byte ch);
};

#endif /*sblib_debug_h*/
