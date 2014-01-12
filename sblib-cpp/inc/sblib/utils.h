/*
 *  utils.h - Utility functions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_utils_h
#define sblib_utils_h

/**
 * Get the offset of a field in a class, structure or type.
 *
 * @param type - the class, structure or type.
 * @param member - the member
 * @return The offset of the member.
 *
 * @brief E.g. for the structure
 *        struct ex
 *        {
 *            char a;
 *            char b
 *            short c;
 *        };
 *
 *        OFFSET_OF(ex,c) returns 2
 */
#define OFFSET_OF(type, field)  ((unsigned int) &(((type *) 0)->field))

#endif /*sblib_utils_h*/
