/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include <sblib/utils.h>
#include "debug_handler.h"
#include "bcu_ft12.h"
#include "config.h"

extern BcuFt12 bcuFt12;

void debugFatal()
{
#ifdef DEBUG
    bcuFt12.setProgPin(LED_KNX_RX);
    fatalError();
#endif
}


/** @}*/
