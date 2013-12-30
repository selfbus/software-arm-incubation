/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sb_utils.h"

unsigned int sb_debounce(unsigned int value, unsigned int time, SbDebounce * debounce)
{
    if (value != debounce->old)
    {
        sb_timer_start(& debounce->debounce_timer, time, 0);
    }
    else if (sb_timer_check(& debounce->debounce_timer))
    {
        debounce->valid = value;
    }
    debounce->old = value;
    return debounce->valid;
}

