/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *                     Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef string_h
#define string_h

/**
 * Copy n bytes from src to dest.
 *
 * @param dest - the destination to copy to
 * @param src - the source to copy from
 * @param n - the number of bytes to copy
 */
void memcpy(void* dest, const void* src, int n);

/**
 * Set n bytes starting at dest to val.
 *
 * @param dest - the destination to set
 * @param val - the value to set
 * @param n - the number of bytes to set
 */
void memset(void* dest, unsigned char val, int n);

#endif /*string_h*/
