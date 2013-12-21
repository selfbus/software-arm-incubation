/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sb_main.h"

#include "sb_bus.h"
#include "sb_proto.h"
#include "internal/sb_hal.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif
#include "gpio.h"


/**
 * Initialize the library. Call this function once when the program starts.
 */
void sb_init()
{
    sb_init_bus();
    sb_init_proto();

    LPC_GPIO[SB_PROG_PORT]->DIR |= 1 << SB_PROG_BIT; // Set prog button+led to output
    LPC_IOCON_PROG = SB_PROG_IOCON;                  // IO configuration for prog button+led
}

/**
 * The lib's main processing. Call this function regularily from your main().
 * It should be called at least every 100ms.
 */
void sb_main_loop()
{
    if (sbRecvTelegramLen)
        sb_process_tel();

    if (!sb_send_ring_empty() && sbSendTelegramLen == 0)
        sb_send_next_tel();

#if 0
    if (sb_eeprom_dirty() && sbStatem == SB_IDLE && !sb_prog_mode_active() && !sb_connected() && sb_send_ring_empty())
        sb_eeprom_write();
#endif

    //
    // Handle programming-mode button and LED
    //
    static unsigned short progButtonLevel = 0;
    const unsigned int progButtonMask = 1 << SB_PROG_BIT;
    LPC_GPIO[SB_PROG_PORT]->DIR &= ~progButtonMask; // Set prog button+led to input
    if (LPC_GPIO[SB_PROG_PORT]->MASKED_ACCESS[progButtonMask] == 0)
    {
        if (progButtonLevel < 255)
        {
            if (++progButtonLevel == 254)
                userram[60] ^= 0x81;  // toggle programming mode and checksum bit
        }
    }
    else if (progButtonLevel > 0)
    {
        --progButtonLevel;
    }

    LPC_GPIO[SB_PROG_PORT]->DIR |= 1 << SB_PROG_BIT; // Set prog button+led to output

    if (userram[60] & 1)
        LPC_GPIO[SB_PROG_PORT]->MASKED_ACCESS[progButtonMask] = 0;
    else LPC_GPIO[SB_PROG_PORT]->MASKED_ACCESS[progButtonMask] = progButtonMask;
}
