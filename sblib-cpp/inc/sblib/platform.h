/*
 *  platform.h - Low level hardware specific stuff.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_platform_h
#define sblib_platform_h

#include <LPC11xx.h>
#include <core_cm0.h>

/**
 * Get a pointer to a low level IO configuration register.
 *
 * @param pin - the IO pin to get the configuration for
 * @return a pointer to the IO configuration register.
 */
unsigned int* ioconPointer(short pin);

/**
 * Low level table of the IO ports
 */
extern LPC_GPIO_TypeDef (* const gpioPorts[4]);


#ifdef IAP_EMULATION
  extern unsigned char FLASH[];
# define __vectors_start__ *FLASH
#else
  extern unsigned int __vectors_start__;
#endif

/**
 * The base address of the flash.
 */
#ifndef FLASH_BASE_ADDRESS
#define FLASH_BASE_ADDRESS ((unsigned char*) &__vectors_start__)
#endif
/**
* The size of a flash sector in bytes.
*/
#define FLASH_SECTOR_SIZE 0x1000


#endif /*sblib_platform_h*/
