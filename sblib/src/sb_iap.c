/*
 *  Copyright (c) 2013 Martin Glück <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#include "sb_iap.h"

typedef enum
{
    IAP_PREPARE = 50,         // Prepare sector(s) for write
    IAP_COPY_RAM2FLASH,     // Copy RAM to Flash
    IAP_ERASE,                 // Erase sector(s)
    IAP_BLANK_CHECK,         // Blank check sector(s)
    IAP_READ_PART_ID,         // Read chip part ID
    IAP_READ_BOOT_VER,         // Read chip boot code version
    IAP_COMPARE,             // Compare memory areas
    IAP_REINVOKE_ISP,         // Reinvoke ISP
    IAP_READ_UID,             // Read unique ID
    IAP_ERASE_PAGE             // Erase page(s)
} IAP_Commands;

typedef enum
{
    CMD_SUCCESS,
    INVALID_COMMAND,
    SRC_ADDR_ERROR,
    DST_ADDR_ERROR,
    SRC_ADDR_NOT_MAPPED,
    DST_ADDR_NOT_MAPPED,
    COUNT_ERROR,
    INVALID_SECTOR,
    SECTOR_NOT_BLANK,
    SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION,
    COMPARE_ERROR,
    BUSY
} IAP_Status;

#define IAP_LOCATION         0x1FFF1FF1
#define SECTOR_SIZE          0x1000

/**
 * Define a contianer for the interface to the IAP function calls
 */
typedef struct
{
    unsigned int cmd;         //!< Command
    unsigned int par[4];      //!< Parameters
    unsigned int stat;        //!< Status
    unsigned int res[4];      //!< Result
} IAP_Parameter;

/* IAP Call */
typedef void (*IAP_Entry) (unsigned int * cmd, unsigned int * stat);

#define IAP_Call ((IAP_Entry) 0x1FFF1FF1)

int sb_iap_erase_sector (int sector)
{
    IAP_Parameter p;

    p.cmd     = IAP_PREPARE;
    p.par [0] = sector;
    p.par [1] = sector;
    IAP_Call (& p.cmd, &p.stat);
    if (p.stat == CMD_SUCCESS)
    {
        p.cmd = IAP_ERASE;
        p.par [0] = sector;
        p.par [1] = sector;
        p.par [2] = SystemCoreClock / 1000;
        IAP_Call (& p.cmd, &p.stat);
        if (p.stat == CMD_SUCCESS)
        {
            p.cmd = IAP_BLANK_CHECK;
            p.par [0] = sector;
            p.par [1] = sector;
            IAP_Call (& p.cmd, &p.stat);
        }
    }
    return p.stat;
}

int sb_iap_program (unsigned int rom, unsigned int ram, unsigned int size)
{
    IAP_Parameter p;
    int           sector = sb_iap_address2sector (rom);
    /* first we need to 'unlock' the sector */
    p.cmd     = IAP_PREPARE;
    p.par [0] = sector;
    p.par [1] = sector;
    IAP_Call (& p.cmd, &p.stat);
    if (p.stat == CMD_SUCCESS)
    {   /* than we can `copy` the RAM content to the FLASH */
        p.cmd = IAP_COPY_RAM2FLASH;
        p.par [0] = rom;
        p.par [1] = ram;
        p.par [2] = size;
        p.par [3] = SystemCoreClock / 1000;
        IAP_Call (& p.cmd, &p.stat);
        if (p.stat == CMD_SUCCESS)
        {
            p.cmd = IAP_COMPARE;
            p.par [0] = rom;
            p.par [1] = ram;
            p.par [2] = size;
            IAP_Call (& p.cmd, &p.stat);
        }
    }
    return p.stat;
}

int sb_iap_read_uid (unsigned int * uid, unsigned int * part_id)
{
    IAP_Parameter p;

    p.cmd     = IAP_READ_PART_ID;
    IAP_Call (& p.cmd, &p.stat);
    * part_id = p.res [0];
    if (p.stat == CMD_SUCCESS)
    {
        p.cmd = IAP_READ_UID;
        IAP_Call (& p.cmd, &p.stat);
        * uid = p.res [0];
    }
    return p.stat;
}

int sb_iap_address2sector (unsigned int address)
{
    return address / SECTOR_SIZE;
}
