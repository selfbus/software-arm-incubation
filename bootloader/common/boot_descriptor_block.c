/*
 * boot_descriptor_block.c
 *
 *  Created on: 10.07.2015
 *      Author: glueck
 */

#include "boot_descriptor_block.h"
#include "crc.h"

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


