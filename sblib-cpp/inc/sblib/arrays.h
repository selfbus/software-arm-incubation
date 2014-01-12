/*
 *  arrays.h - Array handling.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_arrays_h
#define sblib_arrays_h

#include <sblib/types.h>

/**
 * Find a value in an array of integers.
 *
 * @param val - the value to find.
 * @param arr - the array to search.
 * @param count - the number of values in the array.
 * @return The index of the value in the array, -1 if not found.
 */
short indexOf(int val, const int* arr, short count);

/**
 * Find a value in an array of short integers.
 *
 * @param val - the value to find.
 * @param arr - the array to search.
 * @param count - the number of values in the array.
 * @return The index of the value in the array, -1 if not found.
 */
short indexOf(unsigned short val, const unsigned short* arr, short count);

/**
 * Find a value in an array of bytes.
 *
 * @param val - the value to find.
 * @param arr - the array to search.
 * @param count - the number of values in the array.
 * @return The index of the value in the array, -1 if not found.
 */
short indexOf(byte val, const byte* arr, short count);

#endif /*sblib_arrays_h*/
