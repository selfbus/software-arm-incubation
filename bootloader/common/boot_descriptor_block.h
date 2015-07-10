/*
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef BOOT_DESCRIPTOR_BLOCK_H_
#define BOOT_DESCRIPTOR_BLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned int startAddress;
    unsigned int endAddress;
    unsigned int crc;
} AppDescriptionBlock;

unsigned int checkApplication (AppDescriptionBlock * block);

#ifdef __cplusplus
}
#endif

#endif /* BOOT_DESCRIPTOR_BLOCK_H_ */
