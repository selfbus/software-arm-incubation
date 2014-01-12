/*
 *  main.cpp - The library's main.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/main.h>

#include <sblib/interrupt.h>
#include <sblib/timer.h>

#include <sblib/internal/functions.h>
#include <sblib/internal/variables.h>


/**
 * Setup the library.
 */
static inline void lib_setup()
{
    // Configure the system timer to call SysTick_Handler once every 1 msec
    SysTick_Config(SystemCoreClock / 1000);
    systemTime = 0;
}

/**
 * The main of the library.
 *
 * In your program, you will implement setup() and loop(), which are both
 * called by this function.
 */
int main()
{
    lib_setup();
    setup();

    while (1)
    {
        loop();
    }
}
