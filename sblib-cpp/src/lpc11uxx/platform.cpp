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
#include <sblib/digital_pin.h>

#include <sblib/utils.h>

// Get the offset of the pin in the structure LPC_IOCON_Type
#define OFFSET_OF_IOCON(pin)  (OFFSET_OF(LPC_IOCON_Type, pin) >> 2)

// Offsets to IO configurations in the structure LPC_IOCON_TypeDef
static const unsigned short ioconOffsets[2][32] =
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
        OFFSET_OF_IOCON(TDI_PIO0_11),
        OFFSET_OF_IOCON(TMS_PIO0_12),
        OFFSET_OF_IOCON(TDO_PIO0_13),
        OFFSET_OF_IOCON(TRST_PIO0_14),
        OFFSET_OF_IOCON(SWDIO_PIO0_15),
        OFFSET_OF_IOCON(PIO0_16),
        OFFSET_OF_IOCON(PIO0_17),
        OFFSET_OF_IOCON(PIO0_18),
        OFFSET_OF_IOCON(PIO0_19),
        OFFSET_OF_IOCON(PIO0_20),
        OFFSET_OF_IOCON(PIO0_21),
        OFFSET_OF_IOCON(PIO0_22),
        OFFSET_OF_IOCON(PIO0_23),
        0
    },
    {
        OFFSET_OF_IOCON(PIO1_0),
        OFFSET_OF_IOCON(PIO1_1),
        OFFSET_OF_IOCON(PIO1_2),
        OFFSET_OF_IOCON(PIO1_3),
        OFFSET_OF_IOCON(PIO1_4),
        OFFSET_OF_IOCON(PIO1_5),
        OFFSET_OF_IOCON(PIO1_6),
        OFFSET_OF_IOCON(PIO1_7),
        OFFSET_OF_IOCON(PIO1_8),
        OFFSET_OF_IOCON(PIO1_9),
        OFFSET_OF_IOCON(PIO1_10),
        OFFSET_OF_IOCON(PIO1_11),
        OFFSET_OF_IOCON(PIO1_12),
        OFFSET_OF_IOCON(PIO1_13),
        OFFSET_OF_IOCON(PIO1_14),
        OFFSET_OF_IOCON(PIO1_15),
        OFFSET_OF_IOCON(PIO1_16),
        OFFSET_OF_IOCON(PIO1_17),
        OFFSET_OF_IOCON(PIO1_18),
        OFFSET_OF_IOCON(PIO1_19),
        OFFSET_OF_IOCON(PIO1_20),
        OFFSET_OF_IOCON(PIO1_21),
        OFFSET_OF_IOCON(PIO1_22),
        OFFSET_OF_IOCON(PIO1_23),
        OFFSET_OF_IOCON(PIO1_24),
        OFFSET_OF_IOCON(PIO1_25),
        OFFSET_OF_IOCON(PIO1_26),
        OFFSET_OF_IOCON(PIO1_27),
        OFFSET_OF_IOCON(PIO1_28),
        OFFSET_OF_IOCON(PIO1_29),
        OFFSET_OF_IOCON(PIO1_30),
        OFFSET_OF_IOCON(PIO1_31)
    },
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
