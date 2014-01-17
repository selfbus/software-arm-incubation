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

#include <sblib/internal/iap.h>
#include <sblib/eib/bus.h>
#include <sblib/core.h>

#include <string.h>


byte  __attribute__ ((aligned (4))) userRamData[USER_RAM_SIZE];
UserRam& userRam = *(UserRam*) userRamData;

byte  __attribute__ ((aligned (4))) userEepromData[USER_EEPROM_SIZE];
UserEeprom& userEeprom = *(UserEeprom*) userEepromData;

byte userEepromModified;


#define NUM_EEPROM_PAGES  (SB_EEPROM_SECTOR_SIZE / USER_EEPROM_SIZE)
#define EEPROM_PAGE_SIZE  (SB_EEPROM_SECTOR_SIZE / NUM_EEPROM_PAGES)
#define LAST_EEPROM_PAGE  (SB_EEPROM_FLASH_SECTOR_ADDRESS + EEPROM_PAGE_SIZE * (NUM_EEPROM_PAGES - 1))


/*
 * Find the last valid page in the flash sector
 */
byte* findValidPage()
{
    byte* page = LAST_EEPROM_PAGE;

    while (page >= SB_EEPROM_FLASH_SECTOR_ADDRESS)
    {
        if (page[USER_EEPROM_SIZE - 1] != 0xff)
            return page;

        page -= EEPROM_PAGE_SIZE;
    }

    return 0;
}

void readUserEeprom()
{
    byte* page = findValidPage();

    if (page) memcpy(userEepromData, page, USER_EEPROM_SIZE);
    else memset(userEepromData, 0, USER_EEPROM_SIZE);

    userEepromModified = 0;
}

void writeUserEeprom()
{
    if (!userEepromModified)
        return;

    // Wait for an idle bus and then disable the interrupts
    while (!bus.idle())
        ;
    noInterrupts();

    byte* page = findValidPage();
    if (page == LAST_EEPROM_PAGE)
    {
        // Erase the sector
        int sectorId = iapSectorOfAddress(SB_EEPROM_FLASH_SECTOR_ADDRESS);
        IAP_Status rc = iapEraseSector(sectorId);
        if (rc != IAP_SUCCESS) fatalError(); // erasing failed

        page = 0;
    }
    else page += EEPROM_PAGE_SIZE;

    if (!page)
        page = SB_EEPROM_FLASH_SECTOR_ADDRESS;

    userEepromData[USER_EEPROM_SIZE - 1] = 0; // mark the page as in use
    IAP_Status rc = iapProgram(page, userEepromData, USER_EEPROM_SIZE);
    if (rc != IAP_SUCCESS) fatalError(); // flashing failed

    interrupts();
    userEepromModified = 0;
}
