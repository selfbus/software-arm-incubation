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


/**
 * Initialize the library. Call this function once when the program starts.
 */
void sb_init()
{
    sb_init_bus();
    sb_init_proto();
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
}
