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
extern "C" void TIMER16_1_IRQHandler()
{
    // Toggle the pin PIO0_7
    digitalWrite(PIO0_7, !digitalRead(PIO0_7));

    // Clear the timer interrupt flags. Otherwise the interrupt handler is called
    // again immediately after returning.
    timer16_1.flags();
}

/*
 * Initialize the application.
 */
void setup()
{
    pinMode(PIO0_7, OUTPUT);

    // Enable the timer interrupt
    enableInterrupt(TIMER_16_1_IRQn);

    // Begin using the timer
    timer16_1.begin();

    // Let the timer count microseconds
    timer16_1.prescaler((SystemCoreClock / 1000) - 1);

    // On match of MAT1, generate an interrupt and reset the timer
    timer16_1.matchMode(MAT1, RESET | INTERRUPT);

    // Match MAT1 when the timer reaches this value (in milliseconds)
    timer16_1.match(MAT1, 500);

    timer16_1.start();
}

/*
 * The main processing loop.
 */
void loop()
{
    // Sleep until the next interrupt happens
    __WFI();
}
