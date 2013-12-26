/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#define CONFIG_ENABLE_DRIVER_GPIO 1

#include "gpio.h"

#include "sb_main.h"
#include "sb_comobj.h"

#include "sb_bus.h"

#define LED_PORT 0		// Port for led
#define LED_BIT 7		// Bit on port for led

/*
 * Initialize the application.
 */
static inline void init()
{
    sb_init();                        // Initialize the library
    sb_set_appdata(0, 4, 0x7054, 2);  // We are a "Jung 2118" device, version 0.2
    sb_set_wakeup_time(5000);         // Optional: set wakeup time for idle bus to 5msec

    GPIOSetDir(LED_PORT, LED_BIT, 1);
    GPIOSetValue(LED_PORT, LED_BIT, 0);

    // Configure inputs: pull-up, hysteresis
    LPC_IOCON->PIO2_0 = 0x30;
    LPC_IOCON->PIO2_1 = 0x30;
    LPC_IOCON->PIO2_2 = 0x30;
    LPC_IOCON->PIO2_3 = 0x30;
    LPC_IOCON->PIO2_4 = 0x30;
    LPC_IOCON->PIO2_5 = 0x30;
    LPC_IOCON->PIO2_6 = 0x30;
    LPC_IOCON->PIO2_7 = 0x30;
}

/*
 * The application's main.
 */
int main()
{
    int sendTrigger = 0;
    unsigned char testValue = 1;

    init();

   // void* valuePtr = sb_get_value_ptr(8);

    while (1)
    {
        if (sbSendCurTelegram == 0 && sbState == SB_IDLE)
            __WFI();  // goto sleep until an interrupt happens

        sb_main_loop();

        // Send a specific telegram if P1.11 is low
        if (GPIOGetValue(1, 11) == 0)
        {
            if (sendTrigger < 10)
            {
                if (++sendTrigger == 9)
                {
                    GPIOSetValue(LED_PORT, LED_BIT, 1);

                    unsigned char* valuePtr = sb_get_value_ptr(0);
                    *valuePtr = testValue;
                    testValue ^= 1;

                    sb_send_obj_value(0);
                }
            }
        }
        else if (sendTrigger > 0)
        {
            if (!--sendTrigger)
                GPIOSetValue(LED_PORT, LED_BIT, 0);
        }
    }

    return 0;
}
