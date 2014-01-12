/*
 *  print.cpp - Base class that provides print() and println()
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/print.h>

// The size of the internal buffer in print()
#define PRINTBUF_SIZE (8 * sizeof(int) + 1)


int Print::print(int value, Base base, int digits)
{
    int wlen = 0;
    if (value < 0)
    {
        wlen += write('-');
        value = -value;
        --digits;
    }

    return print((unsigned int) value, base, digits) + wlen;
}

int Print::print(unsigned int value, Base base, int digits)
{
    byte buf[PRINTBUF_SIZE]; // need the maximum size for binary printing
    int wlen = 0;
    byte ch;

    short b = (short) base;
    if (b < 2) b = 2;

    byte* pos = buf + PRINTBUF_SIZE;
    do
    {
        ch = value % b;
        *--pos = (ch < 10 ? '0' : 'A' - 10) + ch;

        value /= b;
    }
    while (--digits > 0 || value);

    return write((byte*) pos, buf + (PRINTBUF_SIZE + 1) - pos);
}

int Print::println()
{
    return write('\r') + write('\n');
}

int Print::write(byte ch)
{
    return 0;
}

int Print::write(const byte* data, int count)
{
    int wlen = 0;
    while (count--)
    {
        wlen += write(*data++);
    }

    return wlen;
}

int Print::write(const char* str)
{
    if (!str)
        return 0;

    int len = 0;
    while (str[len])
        ++len;

    return write((const byte*) str, len);
}
