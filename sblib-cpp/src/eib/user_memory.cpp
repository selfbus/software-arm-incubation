/*
 *  user_memory.cpp - BCU user RAM and user EEPROM.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/user_memory.h>


byte  __attribute__ ((aligned (4))) userRamData[USER_RAM_SIZE];
UserRam& userRam = *(UserRam*) userRamData;

byte  __attribute__ ((aligned (4))) userEepromData[USER_EEPROM_SIZE];
UserEeprom& userEeprom = *(UserEeprom*) userEepromData;

byte userEepromModified;


void readUserEeprom()
{
    // TODO implement

    userEepromModified = 0;
}

void writeUserEeprom()
{
    if (!userEepromModified)
        return;

    // TODO implement

    userEepromModified = 0;
}
