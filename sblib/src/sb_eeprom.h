/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

/**
 * @file  sb_eeprom.h
 * @brief EEPROM emulation
 *
 */
#ifndef SB_EEP_EMU_H_
#define SB_EEP_EMU_H_

/**
 * The organizational data for an emulated EEPROM region
 */
typedef struct {
    unsigned char        * ram;         //!< the base address of the RAM mirror
    unsigned short         size;        //!< the size of one page
    unsigned char          rom_pages;   //!< how many pages should be stored in the FLASH
    unsigned char          version;     //!< a layout version ID
    unsigned char          state;       //!< internal state
} SB_Epp;

#define SB_EEPROM_NO_VALID_PAGE_FOUND   -1

extern unsigned char eeprom[];
extern SB_Epp eeprom_r;

/**
 * Initializes the EEPROM region.
 *
 * If a valid page can be found inside the FLASH the data will be copied into
 * the RAM mirror.
 *
 * @param region        handle to the EEPROM region
 * @param clear         specifies if the all sectors belonging to this EEPROM region
 *                      should be erased.
 * @return              Returns the status:
 *                      - 0                          if a valid ROM page has been copied
 *                                                   to the RAM mirror
 *                      - SB_EEPROM_NO_VALID_PAGE_FOUND if no valid FLASH page could be found
 *                      - SB_EEP_VERSION_MISSMATCH   if the version ID did not match
 */
int sb_eeprom_init   (unsigned int  clear);

/**
 * Write the current content of the RAM mirror of the EEPROM region into
 * a new page inside the FLASH.
 * @param region        handle to the EEPROM region
 * @return              status of the operation (0 == OK)
 */
int sb_eeprom_update (void);

#endif /* SB_EEP_EMU_H_ */
