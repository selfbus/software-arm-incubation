/*
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sb_eeprom.h"
#include "sb_iap.h"
#include "sb_bus.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#define EEPROM_SIZE 256

typedef int eep_size_check[(EEPROM_SIZE % 256) == 0 ? 1 : -1];
unsigned char  __attribute__ ((aligned (4))) eeprom[EEPROM_SIZE];
SB_Epp eeprom_r = {eeprom, EEPROM_SIZE, 0x1000 / EEPROM_SIZE};


int sb_eeprom_init(unsigned int clear)
{
    int                   i;
    int                   s  = -1;
    int                   res = SB_EEPROM_NO_VALID_PAGE_FOUND;
    int                   found;
    int                   rom_address;
    const unsigned char * rom;

    eeprom_r.state   = 0xFF; // highest active page number
    rom_address      = SB_EEPROM_FLASH_SECTOR_ADDRESS;

    // if a clear has been forced we need to clear all sectors for this region
    if (clear)
    {
        /* before we erase the Flash we need to disable the interrupts.
         * Wait until the bus module is in the idle state
         */
        while (sbState != SB_IDLE);
        __disable_irq();
        for (i = 0; i < eeprom_r.rom_pages; i++, rom_address += eeprom_r.size)
        {
            int ns = sb_iap_address2sector(rom_address);
            if (s != ns)
            {
                sb_iap_erase_sector(ns);
                s = ns;
            }
        }
        __enable_irq();
    }
    // find the last valid 'page' in the flash area
    i           = eeprom_r.rom_pages - 1;
    found       = 0;
    rom_address = SB_EEPROM_FLASH_SECTOR_ADDRESS + i * eeprom_r.size;
    while (!found && (i >= 0))
    {
    	rom = (unsigned char *) rom_address;
        for (s = 0; !found && (s < eeprom_r.size); s++)
        {
            found = rom [s] != 0xFF;
        }
        rom_address -= eeprom_r.size;
        i           -= !found;
    }
    if (found)
    {
        unsigned int  * lram = (unsigned int *)  eeprom_r.ram;
        unsigned int  * lrom = (unsigned int *) (SB_EEPROM_FLASH_SECTOR_ADDRESS + i * eeprom_r.size);
        eeprom_r.state = i;
        res           = 0;
        /* copy the content of the FLASH into the RAM mirror */
        for (i = 0; i < eeprom_r.size; i += 4)
        {
            * lram++ = * lrom++;
        }
    }
    return res;
}

int sb_eeprom_update()
{
    int res            = 0; // OK
    int page           = eeprom_r.state;
    int current_sector = sb_iap_address2sector (SB_EEPROM_FLASH_SECTOR_ADDRESS + page * eeprom_r.size);
    int erase_required = 0;
    int new_sector;

    if (++page >= eeprom_r.rom_pages)
    {
        /* we mad a wrap around therefore we need to erase the page */
        page           = 0;
        erase_required = 1;
    }
    new_sector      = sb_iap_address2sector(SB_EEPROM_FLASH_SECTOR_ADDRESS + page * eeprom_r.size);
    erase_required |= (new_sector != current_sector);
    if (erase_required)
    {   /* The new page will be in a new sector -> we need to erase
         * that sector.
         * Before we erase the FLash we need to disable the interrupts.
         * Wait until the bus module is in the idle state
         */
        while (sbState != SB_IDLE);
        __disable_irq();
        res = sb_iap_erase_sector (new_sector);
    }
    if (0 == res)
    {   /* now, lets copy the RAM to the new flash page
         * Before we erase the FLash we need to disable the interrupts.
         * Wait until the bus module is in the idle state
         */
       while (sbState != SB_IDLE);
       __disable_irq();
        res      = sb_iap_program
                ( (SB_EEPROM_FLASH_SECTOR_ADDRESS + page * eeprom_r.size)
                , (int) (eeprom_r.ram)
                , eeprom_r.size
                );
        if (0 == res)
        {   // set the new active page number
            eeprom_r.state = page;
        }
    }
    __enable_irq();
    return res;
}


#ifdef EEP_TEST

void sb_eep_test (void)
{
    unsigned int uid, part_id;
    sb_eeprom_init     (0);
    sb_iap_read_uid (& uid, & part_id);
    eeprom[0]     = 0x01;
    eeprom[1]     = 0x23;
    eeprom[2]     = 0x45;
    eeprom[256-5] = 0xEF;
    sb_eeprom_update   ()  ;

    eeprom[0]     = 0xAF;
    eeprom[1]     = 0xFE;
    eeprom[2]     = 0xDE;
    eeprom[256-5] = 0x55;
    sb_eeprom_update   ();
}
#endif /* EEP_TEST */
