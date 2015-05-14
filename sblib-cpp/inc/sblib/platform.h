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

#if defined(__LPC11XX__)
#include <LPC11xx.h>
/**
 * Low level table of the IO ports
 */
extern LPC_GPIO_TypeDef (* const gpioPorts[4]);

#elif defined(__LPC11UXX__)
#include <LPC11Uxx.h>
/**
 * Define the types used by LPC11XX CMIS package
 */
typedef LPC_SSPx_Type LPC_SSP_TypeDef;
typedef LPC_CTxxBx_Type LPC_TMR_TypeDef;
#define LPC_UART LPC_USART
#define LPC_TMR16B0 LPC_CT16B0
#define LPC_TMR16B1 LPC_CT16B1
#define LPC_TMR32B0 LPC_CT32B0
#define LPC_TMR32B1 LPC_CT32B1
#else
#error "Unsupported platform"
#endif
#include <core_cm0.h>

/**
 * Get a pointer to a low level IO configuration register.
 *
 * @param pin - the IO pin to get the configuration for
 * @return a pointer to the IO configuration register.
 */
unsigned int* ioconPointer(int pin);

/**
 * Get a pointer to a low level IO configuration register.
 *
 * @param port - the IO port to get the configuration for
 * @param pinNum - the number of the pin to get the configuration for
 * @return a pointer to the IO configuration register.
 */
unsigned int* ioconPointer(int port, int pinNum);


#ifdef IAP_EMULATION
  extern unsigned char FLASH[];
# define __vectors_start__ *FLASH
#else
  extern unsigned int __vectors_start__;
#endif

/**
 * The base address of the flash.
 */
#define FLASH_BASE_ADDRESS ((unsigned char*) &__vectors_start__)

/**
* The size of a flash sector in bytes.
*/
#define FLASH_SECTOR_SIZE 0x1000


#endif /*sblib_platform_h*/
