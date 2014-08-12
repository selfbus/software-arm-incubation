/*
 *  This example application flashes a LED on digital pin PIO1_10
 *  using the PWM output of the 16bit timer #1.
 *
 *  To test, connect: PIO1_10 --[ 470 ohm resistor ]---[ LED ]--- GND
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/core.h>

/*
 * NOTICE
 *
 * The LED to blink has to be connected to pin PIO2_6
 *
 * We cannot blink the LED on the LPCxpresso board with PWM.
 *
 */

/*
 * Initialize the application.
 */
void setup()
{
    pinMode(PIO2_6, OUTPUT_MATCH);  // configure digital pin PIO2_6 to match MAT1 of timer32 #0

    // Begin using the timer.
    timer32_0.begin();

    timer32_0.prescaler((SystemCoreClock / 1000) - 1); // let the timer count milliseconds
    timer32_0.matchMode(MAT1, SET);  // set the output of PIO2_6 to 1 when the timer matches MAT1
    timer32_0.match(MAT1, 800);      // match MAT1 when the timer reaches this value
    timer32_0.pwmEnable(MAT1);       // enable PWM for match channel MAT1

    // Reset the timer when the timer matches MAT3 and generate an interrupt.
    timer32_0.matchMode(MAT3, RESET | INTERRUPT);
    timer32_0.match(MAT3, 1000);     // match MAT3 after 1000 milliseconds

    timer32_0.start();
}

/*
 * The main processing loop.
 */
void loop()
{
    __WFI(); // sleep until the next interrupt occurs
}
