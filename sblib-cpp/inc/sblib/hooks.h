/*
 *  hooks.h - Predefined callback hooks.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_hooks_h
#define sblib_hooks_h

/**
 * Call when a fatal application error happens. This function will never
 * return and the program LED will blink to indicate the error.
 */
void fatalError();

#endif /*sblib_hooks_h*/
