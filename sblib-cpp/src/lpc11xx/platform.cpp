/*
 *  platform.h - Low level hardware specific stuff.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include <sblib/platform.h>

#include <sblib/core.h>
#include <sblib/digital_pin.h>


LPC_GPIO_TypeDef (* const gpioPorts[4]) = { LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3 };

// Get the offset of the pin in the structure LPC_IOCON_TypeDef
#define OFFSET_OF_IOCON(pin)  (OFFSET_OF(LPC_IOCON_TypeDef, pin) >> 2)

// Offsets to IO configurations in the structure LPC_IOCON_TypeDef
static const unsigned short ioconOffsets[4][12] =
{
    {
        OFFSET_OF_IOCON(RESET_PIO0_0),
        OFFSET_OF_IOCON(PIO0_1),
        OFFSET_OF_IOCON(PIO0_2),
        OFFSET_OF_IOCON(PIO0_3),
        OFFSET_OF_IOCON(PIO0_4),
        OFFSET_OF_IOCON(PIO0_5),
        OFFSET_OF_IOCON(PIO0_6),
        OFFSET_OF_IOCON(PIO0_7),
        OFFSET_OF_IOCON(PIO0_8),
        OFFSET_OF_IOCON(PIO0_9),
        OFFSET_OF_IOCON(SWCLK_PIO0_10),
        OFFSET_OF_IOCON(R_PIO0_11)
    },
    {
        OFFSET_OF_IOCON(R_PIO1_0),
        OFFSET_OF_IOCON(R_PIO1_1),
        OFFSET_OF_IOCON(R_PIO1_2),
        OFFSET_OF_IOCON(SWDIO_PIO1_3),
        OFFSET_OF_IOCON(PIO1_4),
        OFFSET_OF_IOCON(PIO1_5),
        OFFSET_OF_IOCON(PIO1_6),
        OFFSET_OF_IOCON(PIO1_7),
        OFFSET_OF_IOCON(PIO1_8),
        OFFSET_OF_IOCON(PIO1_9),
        OFFSET_OF_IOCON(PIO1_10),
        OFFSET_OF_IOCON(PIO1_11)
    },
    {
        OFFSET_OF_IOCON(PIO2_0),
        OFFSET_OF_IOCON(PIO2_1),
        OFFSET_OF_IOCON(PIO2_2),
        OFFSET_OF_IOCON(PIO2_3),
        OFFSET_OF_IOCON(PIO2_4),
        OFFSET_OF_IOCON(PIO2_5),
        OFFSET_OF_IOCON(PIO2_6),
        OFFSET_OF_IOCON(PIO2_7),
        OFFSET_OF_IOCON(PIO2_8),
        OFFSET_OF_IOCON(PIO2_9),
        OFFSET_OF_IOCON(PIO2_10),
        OFFSET_OF_IOCON(PIO2_11)
    },
    {
        OFFSET_OF_IOCON(PIO3_0),
        OFFSET_OF_IOCON(PIO3_1),
        OFFSET_OF_IOCON(PIO3_2),
        OFFSET_OF_IOCON(PIO3_3),
        OFFSET_OF_IOCON(PIO3_4),
        OFFSET_OF_IOCON(PIO3_5),
        0
    }
};

unsigned int* ioconPointer(int pin)
{
    return (unsigned int*) LPC_IOCON_BASE +
        ioconOffsets[digitalPinToPort(pin)][digitalPinToPinNum(pin)];
}

unsigned int* ioconPointer(int port, int pinNum)
{
    return (unsigned int*) LPC_IOCON_BASE +
        ioconOffsets[port][pinNum];
}
