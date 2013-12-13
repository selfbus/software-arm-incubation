/*
 *  Copyright (c) 2013 Martin Glück <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include "sb_eep_emu.h"
#include "sb_iap.h"

#define EEP_SIZE 256

typedef int eep_size_check[(EEP_SIZE % 256) == 0 ? 1 : -1];
unsigned char  __attribute__ ((aligned (4))) eep [EEP_SIZE];
SB_Epp eep_r = {eep, EEP_SIZE, 0x1000 / EEP_SIZE};


int sb_eep_init (unsigned int clear)
{
    int                   i;
    int                   s  = -1;
    int                   res = SB_EEP_NO_VALID_PAGE_FOUND;
    int                   found;
    int                   rom_address;
    const unsigned char * rom;

    eep_r.state   = 0xFF; // highest active page number
    rom_address   = SB_EEP_FLASH_SECTOR_ADDRESS;

    // if a clear has been forced we need to clear all sectors for this region
    if (clear)
    {
        for (i = 0; i < eep_r.rom_pages; i++, rom_address += eep_r.size)
        {
            int ns = sb_iap_address2sector(rom_address);
            if (s != ns)
            {
                sb_iap_erase_sector(ns);
                s = ns;
            }
        }
    }
    // find the last valid `page` in the flash area
    i           = eep_r.rom_pages - 1;
    found       = 0;
    rom_address = SB_EEP_FLASH_SECTOR_ADDRESS + i * eep_r.size;
    while (!found && (i >= 0))
    {
    	rom = (unsigned char *) rom_address;
        for (s = 0; !found && (s < eep_r.size); s++)
        {
            found = rom [s] != 0xFF;
        }
        rom_address -= eep_r.size;
        i           -= !found;
    }
    if (found)
    {
        unsigned int  * lram = (unsigned int *)  eep_r.ram;
        unsigned int  * lrom = (unsigned int *) (SB_EEP_FLASH_SECTOR_ADDRESS + i * eep_r.size);
        eep_r.state = i;
        res           = 0;
        /* copy the content of the FLASH into the RAM mirror */
        for (i = 0; i < eep_r.size; i += 4)
        {
            * lram++ = * lrom++;
        }
    }
    return res;
}

int sb_eep_update (void)
{
    int res            = 0; // OK
    int page           = eep_r.state;
    int current_sector = sb_iap_address2sector (SB_EEP_FLASH_SECTOR_ADDRESS + page * eep_r.size);
    int erase_required = 0;
    int new_sector;

    if (++page >= eep_r.rom_pages)
    {
        /* we mad a wrap around therefore we need to erase the page */
        page           = 0;
        erase_required = 1;
    }
    new_sector      = sb_iap_address2sector(SB_EEP_FLASH_SECTOR_ADDRESS + page * eep_r.size);
    erase_required |= (new_sector != current_sector);
    if (erase_required)
    {   // the new page will be in a new sector -> we need to erase
        // that sector
        res = sb_iap_erase_sector (new_sector);
    }
    if (0 == res)
    {   // now, lets copy the RAM to the new flash page
        res      = sb_iap_program
                ( (SB_EEP_FLASH_SECTOR_ADDRESS + page * eep_r.size)
                , (int) (eep_r.ram)
                , eep_r.size
                );
        if (0 == res)
        {   // set the new active page number
            eep_r.state = page;
        }
    }
    return res;
}


#ifdef EEP_TEST

void sb_eep_test (void)
{
    unsigned int uid, part_id;
    sb_eep_init     (0);
    sb_iap_read_uid (& uid, & part_id);
    eep [0]     = 0x01;
    eep [1]     = 0x23;
    eep [2]     = 0x45;
    eep [256-5] = 0xEF;
    sb_eep_update   ()  ;

    eep [0]     = 0xAF;
    eep [1]     = 0xFE;
    eep [2]     = 0xDE;
    eep [256-5] = 0x55;
    sb_eep_update   ();
}
#endif /* EEP_TEST */
