/*
 *  A simple application that blinks the LED of the LPCxpresso board (on pin PIO0.7)
 *  using a timer and the timer interrupt.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/core.h>

/*
 * Handler for the timer interrupt.
 */
extern "C" void TIMER32_0_IRQHandler()
{
    // Toggle the pin PIO0_7
    digitalWrite(PIO0_7, !digitalRead(PIO0_7));

    // Clear the timer interrupt flags. Otherwise the interrupt handler is called
    // again immediately after returning.
    timer32_0.resetFlags();
}

/*
 * Initialize the application.
 */
void setup()
{
    pinMode(PIO0_7, OUTPUT);

    // Enable the timer interrupt
    enableInterrupt(TIMER_32_0_IRQn);

    // Begin using the timer
    timer32_0.begin();

    // Let the timer count milliseconds
    timer32_0.prescaler((SystemCoreClock / 1000) - 1);

    // On match of MAT1, generate an interrupt and reset the timer
    timer32_0.matchMode(MAT1, RESET | INTERRUPT);

    // Match MAT1 when the timer reaches this value (in milliseconds)
    timer32_0.match(MAT1, 500);

    timer32_0.start();
}

/*
 * The main processing loop.
 */
void loop()
{
    // Sleep until the next interrupt happens
    __WFI();
}
