/*
 *  digital_pin.h - Functions for digital I/O
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_digital_pin_h
#define sblib_digital_pin_h

#include <sblib/types.h>
#include <sblib/ioports.h>


/**
 * Configure the mode of an I/O pin.
 *
 * @param pin - the pin to configure: PIO0_0, PIO0_1, ...
 * @param mode - the I/O mode to set. Use a combination of the PinMode values (see digital_pin.h)
 */
void pinMode(int pin, int mode);

/**
 * Set the value of a digital output pin.
 *
 * @param pin - the pin to set: PIO0_0, PIO0_1, ...
 * @param value - the value to set: true or false.
 */
void digitalWrite(int pin, boolean value);

/**
 * Read the value of a digital input pin.
 *
 * @param pin - the pin to read: PIO0_0, PIO0_1, ...
 * @return The value of the pin: true (1) or false (0).
 */
boolean digitalRead(int pin);

/**
 * Get the port number of the pin.
 *
 * @param pin - the pin to process, e.g. PIO1_9
 * @return The port number of the pin, e.g. 1
 */
#define digitalPinToPort(pin) ((pin >> 5) & 3)

/**
 * Get the pin number of the pin.
 *
 * @param pin - the pin to process, e.g. PIO1_9
 * @return The pin number of the pin, e.g. 9
 */
#define digitalPinToPinNum(pin) (pin & 31)

/**
 * Get the bit mask for the pin.
 *
 * @param pin - the pin to process, e.g. PIO1_9
 * @return The bit mask for the pin, e.g. 0x200
 */
#define digitalPinToBitMask(pin) portMask[pin & 31]


/**
 * Modes for I/O pin configuration with pinMode().
 */
enum PinMode
{
    /**
     * Configure the pin as standard digital input.
     */
    INPUT = 0x1000,

    /**
     * Configure the pin as capture input for a timer. This only works for the following timer,
     * digital pin, and timer capture register combinations (for LPC11xx):
     *
     * 16 Bit timer #0: capture register CR0: PIO0_2 or PIO3_3.
     *                  capture register CR1: PIO3_4.
     * 16 Bit timer #1: capture register CR0: PIO1_8.
     *                  capture register CR1: PIO3_5.
     * 32 Bit timer #0: capture register CR0: PIO1_5 or PIO2_9.
     *                  capture register CR1: PIO2_11.
     * 32 Bit timer #1: capture register CR0: PIO1_0.
     *                  capture register CR1: PIO1_11.
     *
     * Configuring digital pins as capture input that do not have this function will cause
     * unexpected behavior.
     *
     * If there are multiple pins possible, then the last pin that is configured as
     * INPUT_CAPTURE will have the capture register assigned.
     *
     * For example pin PIO1_8 captures to the capture register CR0 of the 16 bit timer #1
     * if activated.
     */
    INPUT_CAPTURE = 0x2000,

    /**
     * Configure the pin as analog input.
     */
    INPUT_ANALOG = 0x3000,

    /**
     * Enable the pull-up resistor for input.
     */
    PULL_UP = 0x10,

    /**
     * Enable the pull-down resistor for input.
     */
    PULL_DOWN = 0x08,

    /**
     * Enable the input hysteresis.
     */
    HYSTERESIS = 0x20,

    /**
     * Configure the pin as standard digital output.
     */
    OUTPUT = 0x6000,

    /**
     * Configure the pin as match output for a timer. This option is used to configure
     * a digital pin for PWM output. Match output is available for the following combinations
     * of timer, match register, and digital pin:
     *
     * 16 Bit timer #0:  match register MR0 for pin PIO0_8 or PIO3_0,
     *                   match register MR1 for pin PIO0_9,
     *                   match register MR2 for pin PIO0_10 or PIO3_2.
     * 16 Bit timer #1:  match register MR0 for pin PIO1_9,
     *                   match register MR1 for pin PIO1_10 or PIO2_4.
     * 32 Bit timer #0:  match register MR0 for pin PIO1_6 or PIO2_5,
     *                   match register MR1 for pin PIO1_7 or PIO2_6,
     *                   match register MR2 for pin PIO0_1 or PIO2_7,
     *                   match register MR3 for pin PIO0_11 or PIO2_8.
     * 32 Bit timer #1:  match register MR0 for pin PIO1_1,
     *                   match register MR1 for pin PIO1_2,
     *                   match register MR2 for pin PIO1_3,
     *                   match register MR3 for pin PIO1_4.
     *
     * Configuring digital pins as output match that do not have this function will cause
     * unexpected behavior.
     *
     * For example pin PIO1_9 is driven by the match register MR0 of the 16 bit
     * timer #1 if activated.
     */
    OUTPUT_MATCH = 0x7000,

    /**
     * Enable open-drain output mode. If not enabled, the standard output mode is used.
     */
    OPEN_DRAIN = 0x400,

    /**
     * Enable the repeater mode.
     */
    REPEATER_MODE = 0x18
};


/**
 * Select a specific port pin function when setting the pin mode.
 * This macro is intended to be used in combination with pinMode().
 *
 * Example: pinMode(PIO1_6, INPUT | PINMODE_FUNC(PF_RXD));
 * This enables the UART RXD function on pin PIO1_6.
 */
#define PINMODE_FUNC(f) ((f) << 18)

#endif /*sblib_digital_pin_h*/
