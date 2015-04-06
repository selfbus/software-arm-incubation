/*
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/internal/iap.h>

#include <sblib/interrupt.h>
#include <sblib/platform.h>
#include <string.h>

// The maximum memory that is tested when searching for the flash size, in bytes
#define MAX_FLASH_SIZE 0x80000

// The increments when searching for the flash size
#define FLASH_SIZE_SEARCH_INC 0x2000


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

// The size of the flash in bytes. Use iapFlashSize() to get the flash size.
int iapFlashBytes = 0;


/** 
 * IAP call function (DO NOT USE UNLESS YOU KNOW WHAT YOU ARE DOING!)
 * use instead: IAP_Call_InterruptSafe()
 */
typedef void (*IAP_Func)(unsigned int * cmd, unsigned int * stat);

#ifndef IAP_EMULATION
#  if defined(__LPC11XX__) || defined(__LPC13XX__) || defined(__LPC17XX__)
#    define IAP_LOCATION      0x1FFF1FF1
#  elif defined(__LPC2XXX__)
#    define IAP_LOCATION      0x7FFFFFF1
#  else
#    error "Unsupported processor"
#  endif
#  define IAP_Call ((IAP_Func) IAP_LOCATION)
#else
   extern "C" void IAP_Call (unsigned int * cmd, unsigned int * stat);
#endif


/**
 * IAP_Call_InterruptSafe(): interrupt-safe IAP_Call function
 *
 * ATTENTION: interrupts shall be blocked during an IAP_Call()!
 *
 * Reason: during an IAP_Call() with flash access the flash is inaccessible for
 *         the user application. When an interrupt occurs and the Interrupt
 *         Vector Table is located in the Flash this will fail and raise a
 *         non-handled HardFault condition.
 */
inline void IAP_Call_InterruptSafe(unsigned int *cmd, unsigned int *stat)
{
    noInterrupts();
    IAP_Call(cmd, stat);
    interrupts();
}


IAP_Status iapEraseSector(int sector)
{
    IAP_Parameter p;

    p.cmd = CMD_PREPARE;
    p.par[0] = sector;
    p.par[1] = sector;
    IAP_Call_InterruptSafe(&p.cmd, &p.stat);

    if (p.stat == IAP_SUCCESS)
    {
        p.cmd = CMD_ERASE;
        p.par[0] = sector;
        p.par[1] = sector;
        p.par[2] = SystemCoreClock / 1000;
        IAP_Call_InterruptSafe(&p.cmd, &p.stat);

        if (p.stat == IAP_SUCCESS)
        {
            p.cmd = CMD_BLANK_CHECK;
            p.par[0] = sector;
            p.par[1] = sector;
            IAP_Call_InterruptSafe(&p.cmd, &p.stat);
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
    IAP_Call_InterruptSafe(&p.cmd, &p.stat);

    if (p.stat == IAP_SUCCESS)
    {
        /* then we can `copy` the RAM content to the FLASH */
        p.cmd = CMD_COPY_RAM2FLASH;
        p.par[0] = (unsigned int) rom;
        p.par[1] = (unsigned int) ram;
        p.par[2] = size;
        p.par[3] = SystemCoreClock / 1000;
        IAP_Call_InterruptSafe(&p.cmd, &p.stat);

        if (p.stat == IAP_SUCCESS)
        {
            p.cmd = CMD_COMPARE;
            p.par[0] = (unsigned int) rom;
            p.par[1] = (unsigned int) ram;
            p.par[2] = size;
            IAP_Call_InterruptSafe(&p.cmd, &p.stat);
        }
    }
    return (IAP_Status) p.stat;
}

IAP_Status iapReadUID(byte* uid)
{
    IAP_Parameter p;
    p.cmd = CMD_READ_UID;

    IAP_Call_InterruptSafe(&p.cmd, &p.stat);
    memcpy(uid, p.res, 16);

    return (IAP_Status) p.stat;
}

IAP_Status iapReadPartID(unsigned int* partId)
{
    IAP_Parameter p;
    p.cmd = CMD_READ_PART_ID;

    IAP_Call_InterruptSafe(&p.cmd, &p.stat);
    *partId = p.res[0];

    return (IAP_Status) p.stat;
}

int iapSectorOfAddress(const byte* address)
{
    return (address - FLASH_BASE_ADDRESS) / FLASH_SECTOR_SIZE;
}

int iapFlashSize()
{
    if (iapFlashBytes)
        return iapFlashBytes;

    IAP_Parameter p;
    p.cmd = CMD_BLANK_CHECK;

    const int sectorInc = FLASH_SIZE_SEARCH_INC / FLASH_SECTOR_SIZE;
    int sector = sectorInc;
    const int maxSector = MAX_FLASH_SIZE / FLASH_SECTOR_SIZE;

    while (sector < maxSector)
    {
        p.par[0] = sector;
        p.par[1] = sector;
        IAP_Call_InterruptSafe(&p.cmd, &p.stat);

        if (p.stat == IAP_INVALID_SECTOR)
            break;

        sector += sectorInc;
    }

    iapFlashBytes = sector * FLASH_SECTOR_SIZE;
    return iapFlashBytes;
}
