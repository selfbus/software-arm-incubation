/*
 *  iap_emu.h - Emulation of the In App Programming functions
 *
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef IAP_EMU_H_
#define IAP_EMU_H_

#ifdef __cplusplus
extern "C" {
#endif


#define I_PREPARE         0
#define I_ERASE           1
#define I_BLANK_CHECK     2
#define I_RAM2FLASH       3
#define I_COMPARE         4

extern int iap_calls [5];
void IAP_Init_Flash(unsigned char value);

#ifdef __cplusplus
}
#endif

#endif /* IAP_EMU_H_ */
