/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_in8.h"
#include "driver_config.h"

#include "sb_main.h"
#include "sb_comobj.h"
#include "sb_bus.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#include "gpio.h"


#define LED_PORT 0		// Port for led
#define LED_BIT 7		// Bit on port for led

/**
 * Initialize global stuff.
 */
static inline void init()
{
    GPIOSetDir(LED_PORT, LED_BIT, 1);
    GPIOSetValue(LED_PORT, LED_BIT, 0);
}

/**
 * The application's main.
 */
int main()
{
    sb_init();                       // Initialize the library
    sb_set_appdata(0, 4, 0x7054, 2); // We are a "Jung 2118" device, version 0.2
    sb_set_wakeup_time(5000);        // Optional: set wakeup time (default is 1 msec)

    init();
    app_init();

    while (1)
    {
        if (sb_idle())
            __WFI();              // Sleep (until the next wakeup time)

        sb_main_loop();           // The lib's main loop
        handle_inputs();          // Handle the input pins
        handle_comobj_updates();  // Handle updates of the com-objects
    }

    return 0;
}
