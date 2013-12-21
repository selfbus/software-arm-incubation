/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
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


/**
 * Set manufacturer data, manufacturer-ID, and device type.
 *
 * @param data - the manufacturer data
 * @param manufacturer - the manufacturer ID
 * @param deviceType - the device type
 */
void sb_set_appdata(unsigned short data, unsigned short manufacturer, unsigned short deviceType)
{
    sbEeprom->manuDataH = data >> 8;
    sbEeprom->manuDataL = data;

    sbEeprom->manufacturerH = manufacturer >> 8;
    sbEeprom->manufacturerL = manufacturer;

    sbEeprom->deviceTypeH = deviceType >> 8;
    sbEeprom->deviceTypeL = deviceType;
}
