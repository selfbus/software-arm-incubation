/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "sb_memory.h"

unsigned char sbUserRamData[SB_USERRAM_SIZE];
SbUserRam *sbUserRam = (SbUserRam*) sbUserRamData;

unsigned char sbEepromData[SB_USERRAM_SIZE];
SbEeprom *sbEeprom = (SbEeprom*) sbEepromData;
