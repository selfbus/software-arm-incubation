/*
 *  bcu.h - BCU specific stuff.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/bcu.h>


UserRam userRam;
byte* userRamData = (byte*) &userRam;

UserEeprom userEeprom;
byte* userEepromData = (byte*) &userEeprom;
