/*
 *  Copyright (c) 2013 Martin Glück <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include "sb_eep_emu.h"
#include "sb_iap.h"

#define PAGE_VALID(region, address) (address [region->size - 1]  != 0xFF)

int sb_eep_init (SB_Epp_Region * region)
{
                   int    i;
    const unsigned char * rom;

    region->flags = 0xF0; // highest active page number
    rom           = region->rom_base;

    /* find the last valid `page` in the flash area */
    i = 0;
    while (PAGE_VALID (region, rom) && (i < region->rom_pages))
    {
        rom += region->size;
        i++;
    }
    i--;
    if (i >= 0)
    {
        region->flags = SB_EEP_RAM_VALID | (i << 4);
        /* copy the content of the FLASH into the RAM mirror */
        unsigned int * lram = (unsigned int *)  region->ram;
        unsigned int * lrom = (unsigned int *) (region->rom_base + i * region->size);
        for (i = 0; i < region->size; i += 4)
        {
            * lram++ = * lrom++;
        }
    }
    return region->flags;
}

int sb_eep_update (SB_Epp_Region * region)
{
    int res            = 0; // OK
    int page           = (region->flags >> 4) & 0x0F;
    int current_sector = sb_iap_address2sector (((int) region->rom_base) + page * region->size);
    int erase_required = 0;
    int new_sector;

    if (++page > region->rom_pages)
    {
        /* we mad a wrap around therefore we need to erase the page */
        page           = 0;
        erase_required = 1;
    }
    new_sector      = sb_iap_address2sector((int)region->rom_base + page * region->size);
    erase_required |= (new_sector != current_sector);
    if (erase_required)
    {   // the new page will be in a new sector -> we need to erase
        // that sector
        res = sb_iap_erase_sector (new_sector);
    }
    if (0 == res)
    {   // now, lets copy the RAM to the new flash page
        // mark the new page as valid
        region->ram [region->size - 1] = 0x00;
        res = sb_iap_program
                ( (unsigned int) (region->rom_base + page * region->size)
                , (unsigned int)  region->ram
                , region->size
                );
        if (0 == res)
        {   // set the new active page number
            region->flags = (region->flags & 0x0F) | (page << 4);
        }
    }
    return res;
}

#define TEST

#ifdef TEST

SB_DEFINE_REGION (test_1, 252,4)
SB_DEFINE_REGION (test_2, 508,2)

void sb_eep_test (void)
{
    unsigned int uid, part_id;
    sb_eep_init     (& test_1);
    sb_eep_init     (& test_2);
    sb_iap_read_uid (& uid, & part_id);
    test_1.ram [0]     = 0x01;
    test_1.ram [1]     = 0x23;
    test_1.ram [2]     = 0x45;
    test_1.ram [256-5] = 0xEF;
    sb_eep_update   (& test_1);

    test_1.ram [0]     = 0xAF;
    test_1.ram [1]     = 0xFE;
    test_1.ram [2]     = 0xDE;
    test_1.ram [256-5] = 0x55;
    sb_eep_update   (& test_1);
}
#endif
