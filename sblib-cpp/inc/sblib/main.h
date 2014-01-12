/*
 *  main.h - Functions for the main application.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_main_h
#define sblib_main_h

/**
 * This function is called by the library's main when the processor is
 * started or reset. Implement this function in your code.
 */
void setup();

/**
 * This function is called in a loop by the library's main. This is where
 * your main work shall happen. The loop() function must return after doing
 * its work. Implement this function in your code.
 */
void loop();

#endif /*sblib_main_h*/
