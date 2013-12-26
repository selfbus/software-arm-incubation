/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef com_objs_h
#define com_objs_h

/**
 * Application version of the user-ram
 */
typedef struct AppUserRam
{
    unsigned char data1[0x60];
    unsigned char status;         // 0x0060: System status
    unsigned char data2[0x3f];

} AppUserRam;

#endif /*com_objs_h*/
