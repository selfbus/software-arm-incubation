/*
 *  BootLoader.c - The bootloader.
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <crc.h>

typedef struct
{
    unsigned int startAddress;
    unsigned int endAddress;
    unsigned int crc;
} AppDescriptionBlock;

const unsigned int blockAddresses[] = { 0x1000 - 0x200, 0x1000 - 0x100, 0x0 };

unsigned int checkVectorTable (unsigned int start)
{
    unsigned int i;
    unsigned int * address;
    unsigned int cs = 0;
    address = (unsigned int *) start;
    for (i = 0; i < 8;i++, address++)
        cs += * address;
    return cs == 0;
}

unsigned int checkApplication (AppDescriptionBlock * block)
{
    unsigned int crc = crc32
        ( 0xFFFFFFFF
        , (unsigned char *) block->startAddress
        , block->endAddress - block->startAddress
        );
    if (crc == block->crc)
    {
        return checkVectorTable(block->startAddress);
    }
    return 0;
}

void jumpToApplication(unsigned int start)
{
    unsigned int ResetVector = * (unsigned int *) (start + 4);
    unsigned int * rom = (unsigned int *) start;
    unsigned int * ram = (unsigned int *) 0x10000000;
    unsigned int i;
    // copy the first 200 bytes of the "application" (the vector table)
    // into the RAM and than remap the vector table inside the RAM
    for (i = 0; i < 50; i++, rom++, ram++)
        * ram = * rom;
    LPC_SYSCON->SYSMEMREMAP = 0x01;

    void (*fptr)(void);
    fptr = (void (*)(void))ResetVector;
    fptr();
}

int main (void)
{
    unsigned int * blockAddr = (unsigned int *) blockAddresses;
    while (* blockAddr)
    {
        AppDescriptionBlock * block = (AppDescriptionBlock *) (* blockAddr);
        if (checkApplication(block))
            jumpToApplication(block->startAddress);
        blockAddr++;
    }
    return 0 ;
}
