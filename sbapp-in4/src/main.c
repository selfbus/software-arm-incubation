/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
 *                     Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_in4.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#define CONFIG_ENABLE_DRIVER_GPIO 1

#include "gpio.h"
#include "uart.h"

#include "sb_bus.h"
#include "sb_proto.h"
#include "sb_main.h"
#include "sb_memory.h"
#include "sb_timer.h"


int main()
{
    sb_init();
    sb_set_appdata(0, 2, 0x9009, 0x01);  // we are a ABB TSU/4.2 version 0.1
    sb_set_wakeup_time(5000);

    app_init();

    while (1)
    {
        if (sb_bus_idle())
            __WFI();              // Sleep (until the next wakeup time)

        sb_main_loop();           // The lib's main loop
//        handle_inputs();          // Handle the input pins
//        handle_comobj_updates();  // Handle updates of the com-objects
    }
}
