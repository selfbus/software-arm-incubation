/*
 *  A simple example for SPI.
 *
 *  This example configures SPI for output and sends a byte every second.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/core.h>
#include <sblib/eib.h>

#include <sblib/internal/iap.h>
#include <sblib/ioports.h>
#include <sblib/spi.h>


/*
 * We use SPI port 0 in this example.
 *
 * Pinout:
 *
 * PIO0_2:  SSEL0
 * PIO0_9:  MOSI0
 * PIO2_11: SCK0
 */


SPI spi(SPI_PORT_0);

int blinkPin = PIO2_6;


/*
 * Initialize the application.
 */
void setup()
{
	bcu.begin(4, 0x7054, 2); // We are a "Jung 2118" device, version 0.2

	pinMode(blinkPin,  OUTPUT);
    pinMode(PIO3_3, OUTPUT);	// Green LED

    // SPI
    pinMode(PIO0_2,  OUTPUT);
    pinMode(PIO0_9,  OUTPUT | SPI_MOSI);
    pinMode(PIO2_11, OUTPUT | SPI_CLOCK);
    pinMode(PIO0_8, INPUT | SPI_MISO);

    // ADC Pins
    pinMode(PIO1_5, OUTPUT);


    spi.setClockDivider(128);
    spi.begin();

    digitalWrite(PIO3_3, true);

    serial.begin(19200);

	serial.println("Selfbus serial port example");

	serial.print("Target MCU has ");
	serial.print(iapFlashSize() / 1024);
	serial.println("k flash");
	serial.println();
}

/*
 * The main processing loop.
 */
void loop()
{
	volatile int i;
	static int val = 0;
	volatile char adc_1;

    val++;
    val &= 255;

    digitalWrite(PIO1_5, true);
    delay(1);
    digitalWrite(PIO1_5, false);
    delay(1);

    digitalWrite(blinkPin, true);
    adc_1=spi.transfer(val);
    delay(200);

    digitalWrite(blinkPin, false);
    delay(800);

    serial.print("ADC value: ");
    serial.println(adc_1, HEX, 4);

    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}
