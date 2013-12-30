/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#ifndef SB_UTILS_H_
#define SB_UTILS_H_

#include "sb_timer.h"

#define SB_DEBOUNCE_10MS    (10000)

typedef struct
{
    unsigned int valid;
    unsigned int old;
    SbTimer      debounce_timer;
} SbDebounce;

unsigned int sb_debounce(unsigned int value, unsigned int time, SbDebounce * debounce);

#endif /* SB_UTILS_H_ */
