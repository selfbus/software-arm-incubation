/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sb_memory.h"
#include "sb_eeprom.h"

unsigned char __attribute__ ((aligned (4))) sbUserRamData[SB_USERRAM_SIZE];
SbUserRam *sbUserRam = (SbUserRam*) sbUserRamData;

unsigned char *sbEepromData = (unsigned char*) eeprom;
SbEeprom *sbEeprom = (SbEeprom*) eeprom;

static SbGlobal sbGlobalData;
SbGlobal *sbGlobal = &sbGlobalData;
