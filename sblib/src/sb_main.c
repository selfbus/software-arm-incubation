/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sb_main.h"

#include "sb_bus.h"
#include "sb_comobj.h"
#include "sb_eeprom.h"
#include "sb_memory.h"
#include "sb_proto.h"
#include "sb_timer.h"
#include "sb_utils.h"
#include "internal/sb_hal.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#include "gpio.h"


// Debouncer for the prog button
static SbDebounce sbProgDebounce;


/**
 * Initialize the library. Call this function once when the program starts.
 */
void sb_init()
{
    sb_set_wakeup_time(1000);

    sb_eeprom_init(0);
    sb_init_bus();
    sb_init_proto();

    sb_init_debounce(&sbProgDebounce, 0);
    LPC_GPIO[SB_PROG_PORT]->DIR |= 1 << SB_PROG_BIT; // Set prog button+led to output
    LPC_IOCON_PROG = SB_PROG_IOCON;                  // IO configuration for prog button+led

    sbUserRam->status = 0x2e;
    sbUserRam->progRunning = 1;

#ifdef SB_BCU2
    sbEeprom->appType = 0;  // Set to BCU2 application. ETS reads this when programming.
#endif
}

/**
 * The lib's main processing. Call this function regularily from your main().
 * It should be called at least every 100ms.
 */
void sb_main_loop()
{
    do // Loop as long as prog button is pressed or the application program is stopped
    {
        if (sbRecvTelegramLen && sbSendCurTelegram == 0)
            sb_process_tel();

        if (sbSendCurTelegram == 0)
            sb_send_next_tel();

        //
        // Handle programming-mode button and LED
        //
        LPC_GPIO[SB_PROG_PORT]->DIR &= ~SB_PROG_MASK; // Set prog button+led to input
        unsigned char buttonPressed = !LPC_GPIO[SB_PROG_PORT]->MASKED_ACCESS[SB_PROG_MASK];

        // Detect the rising edge of pressing the prog button
        unsigned char oldValue = sbProgDebounce.value;
        if (sb_debounce(buttonPressed, SB_DEBOUNCE_10MS, &sbProgDebounce) && !oldValue)
            sbUserRam->status ^= 0x81;  // toggle programming mode and checksum bit

        LPC_GPIO[SB_PROG_PORT]->DIR |= 1 << SB_PROG_BIT; // Set prog button+led to output

        if (sb_prog_mode_active())
            LPC_GPIO[SB_PROG_PORT]->MASKED_ACCESS[SB_PROG_MASK] = 0;
        else LPC_GPIO[SB_PROG_PORT]->MASKED_ACCESS[SB_PROG_MASK] = SB_PROG_MASK;
    }
    while (sb_prog_mode_active() || !sbUserRam->progRunning);

    if (sbEepromDirty && sb_bus_idle() && !sb_connected())
    {
        sbEepromDirty = 0;
        sb_eeprom_update();
    }
}

/**
 * Set manufacturer data, manufacturer-ID, and device type.
 *
 * @param data - the manufacturer data
 * @param manufacturer - the manufacturer ID
 * @param deviceType - the device type
 * @param version - the version of the application program
 */
void sb_set_appdata(unsigned short data, unsigned short manufacturer, unsigned short deviceType,
                    unsigned char version)
{
    sbEeprom->manuDataH = data >> 8;
    sbEeprom->manuDataL = data;

    sbEeprom->manufacturerH = manufacturer >> 8;
    sbEeprom->manufacturerL = manufacturer;

    sbEeprom->deviceTypeH = deviceType >> 8;
    sbEeprom->deviceTypeL = deviceType;

    sbEeprom->version = version;
}
