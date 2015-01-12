/*
 *  A simple application that sends pulses on a digital port. Per default
 *  PIO0_7 is used, which is the LED on the LPCxpresso board.
 *
 *  This example is meant to be a test for the timing of delayMicroseconds().
 *
 *  Copyright (c) 2015 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/core.h>

// The pin that is used for the example
int pin = PIO0_7;

int mask = digitalPinToBitMask(pin);
LPC_GPIO_TypeDef* port = gpioPorts[digitalPinToPort(pin)];


/*
 * Initialize the application.
 */
void setup()
{
    pinMode(pin, OUTPUT);
}

/*
 * The main processing loop.
 */
void loop()
{
    /* To be as accurate as possible, we directly access the IO pin and do not
     * use digitalWrite().
     *
     * This is for exact testing of delayMicroseconds() and should not be used
     * in your own code!
     */
    port->MASKED_ACCESS[mask] = mask;
    delayMicroseconds(10);
    port->MASKED_ACCESS[mask] = 0;
    delayMicroseconds(50);

    port->MASKED_ACCESS[mask] = mask;
    delayMicroseconds(100);
    port->MASKED_ACCESS[mask] = 0;
    delayMicroseconds(500);

    port->MASKED_ACCESS[mask] = mask;
    delayMicroseconds(1000);
    port->MASKED_ACCESS[mask] = 0;
    delayMicroseconds(5000);

    port->MASKED_ACCESS[mask] = mask;
    delayMicroseconds(10000);
    port->MASKED_ACCESS[mask] = 0;
    delayMicroseconds(50000);

    port->MASKED_ACCESS[mask] = mask;
    delayMicroseconds(100000);
    port->MASKED_ACCESS[mask] = 0;
    delay(500);
}
