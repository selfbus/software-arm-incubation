/*
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/internal/iap.h>

#include <sblib/platform.h>

#if defined(__LPC11XX__) || defined(__LPC13XX__) || defined(__LPC17XX__)
#  define IAP_LOCATION      0x1FFF1FF1
#elif defined(__LPC2XXX__)
#  define IAP_LOCATION      0x7FFFFFF1
#else
#  error "Unsupported processor"
#endif

/**
 * IAP command codes.
 */
enum IAP_Command
{
    CMD_PREPARE = 50,    //!< Prepare sector(s) for write
    CMD_COPY_RAM2FLASH,  //!< Copy RAM to Flash
    CMD_ERASE,           //!< Erase sector(s)
    CMD_BLANK_CHECK,     //!< Blank check sector(s)
    CMD_READ_PART_ID,    //!< Read chip part ID
    CMD_READ_BOOT_VER,   //!< Read chip boot code version
    CMD_COMPARE,         //!< Compare memory areas
    CMD_REINVOKE_ISP,    //!< Reinvoke ISP
    CMD_READ_UID,        //!< Read unique ID
    CMD_ERASE_PAGE       //!< Erase page(s)
};

/**
 * A container for the interface to the IAP function calls.
 */
struct IAP_Parameter
{
    unsigned int cmd;         //!< Command
    unsigned int par[4];      //!< Parameters
    unsigned int stat;        //!< Status
    unsigned int res[4];      //!< Result
};


/** IAP call function */
typedef void (*IAP_Func)(unsigned int * cmd, unsigned int * stat);

#ifndef IAP_EMULATION
#  define IAP_Call ((IAP_Func) 0x1FFF1FF1)
#else
   extern void IAP_Call (unsigned int * cmd, unsigned int * stat);
#endif


IAP_Status iapEraseSector(int sector)
{
    IAP_Parameter p;

    p.cmd = CMD_PREPARE;
    p.par[0] = sector;
    p.par[1] = sector;
    IAP_Call (&p.cmd, &p.stat);

    if (p.stat == IAP_SUCCESS)
    {
        p.cmd = CMD_ERASE;
        p.par[0] = sector;
        p.par[1] = sector;
        p.par[2] = SystemCoreClock / 1000;
        IAP_Call (&p.cmd, &p.stat);

        if (p.stat == IAP_SUCCESS)
        {
            p.cmd = CMD_BLANK_CHECK;
            p.par[0] = sector;
            p.par[1] = sector;
            IAP_Call (&p.cmd, &p.stat);
        }
    }
    return (IAP_Status) p.stat;
}

IAP_Status iapProgram(byte* rom, const byte* ram, unsigned int size)
{
    IAP_Parameter p;
    int sector = iapSectorOfAddress(rom);

    /* first we need to 'unlock' the sector */
    p.cmd = CMD_PREPARE;
    p.par[0] = sector;
    p.par[1] = sector;
    IAP_Call (&p.cmd, &p.stat);

    if (p.stat == IAP_SUCCESS)
    {
        /* then we can `copy` the RAM content to the FLASH */
        p.cmd = CMD_COPY_RAM2FLASH;
        p.par[0] = (unsigned int) rom;
        p.par[1] = (unsigned int) ram;
        p.par[2] = size;
        p.par[3] = SystemCoreClock / 1000;
        IAP_Call (&p.cmd, &p.stat);

        if (p.stat == IAP_SUCCESS)
        {
            p.cmd = CMD_COMPARE;
            p.par[0] = (unsigned int) rom;
            p.par[1] = (unsigned int) ram;
            p.par[2] = size;
            IAP_Call (&p.cmd, &p.stat);
        }
    }
    return (IAP_Status) p.stat;
}

IAP_Status iapReadUID(unsigned int* uid, unsigned int* partId)
{
    IAP_Parameter p;

    p.cmd = CMD_READ_PART_ID;
    IAP_Call (&p.cmd, &p.stat);
    *partId = p.res[0];

    if (p.stat == IAP_SUCCESS)
    {
        p.cmd = CMD_READ_UID;
        IAP_Call (&p.cmd, &p.stat);
        *uid = p.res[0];
    }
    return (IAP_Status) p.stat;
}

int iapSectorOfAddress(const byte* address)
{
    return (address - SB_FLASH_BASE_ADDRESS ) / SB_EEPROM_SECTOR_SIZE;
}
