/*
 *  analog_pin.h - Functions for analog I/O
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_analog_pin_h
#define sblib_analog_pin_h

#include <sblib/ioports.h>
#include <sblib/types.h>

/**
 * Enable the AD converter. This function must be called before analogRead()
 * can be used. It powers on the AD converter unit.
 */
void analogBegin();

/**
 * Power down the AD converter. Call this function when AD conversion is not
 * needed to conserve power.
 */
void analogEnd();

/**
 * Read the value of the specified analog input channel.
 * Call analogBegin() once before using this function.
 *
 * @param channel - the analog channel to read from: AD0, AD1, ... AD7
 * @return The read value (0..1023).
 *
 * @brief The ARM processor has a A/D converter with 10 bit resolution that
 * can read from 8 pins. The following pins can be used as analog input:
 * PIO0_11 (channel AD0), PIO1_0 (AD1), PIO1_1 (AD2), PIO1_2 (AD3), PIO1_3 (AD4),
 * PIO1_4 (AD5), PIO1_10 (AD6), PIO1_11 (AD7).
 *
 * The pin to read from shall be set to analog input mode with:
 * pinMode(pin, ANALOG_INPUT)
 */
int analogRead(int channel);

/**
 * Write an analog value to a pin.
 *
 * NOT YET IMPLEMENTED.
 *
 * @param pin - the pin to write to: PIN0_1, ....
 * @param val - the value to write (0..255)
 *
 * @brief Analog output is done by using PWM output. PWM output occupies the
 * corresponding timer. When no PWM output is available for the selected pin,
 * the function falls back to digital output (low for 0, high for 255).
 *
 * PWM output is available for the following pins:
 * PIO0_1, PIO0_8, PIO0_9, PIO0_10, PIO0_11, PIO1_1, PIO1_2, PIO1_3, PIO1_4,
 * PIO1_6, PIO1_7, PIO1_9, PIO1_10, PIO2_4, PIO2_5, PIO2_6, PIO2_7, PIO2_8,
 * PIO3_0, PIO3_1, PIO3_2.
 */
//void analogWrite(int pin, byte val);

#endif /*sblib_analog_pin_h*/
