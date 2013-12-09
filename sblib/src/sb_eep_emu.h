/*
 *  Copyright (c) 2013 Martin Glück <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */


#ifndef SB_EEP_EMU_H_
#define SB_EEP_EMU_H_

/**
 * Define a EEPROM emulation region with the specified size.
 * @param name      name of the EEPROM region
 * @param size      the size of ONE page (size + 4 must be a multiple of 256)
 * @param copies    how many pages should be allocated of this reagion
 */
#define SB_DEFINE_REGION(name,size,copies)                               \
    typedef int name##_size_check[((size + 4) % 256) == 0 ? 1 : -1];     \
       unsigned char  __attribute__ ((aligned (4))) name##_m [size + 4]; \
  __attribute__ ((section(".eeprom_data")))                              \
  const unsigned char name##_f [(size + 4) * copies];                    \
        SB_Epp_Region name = {name##_m, name##_f, 4 + size, copies};

/**
 * The organisational data for an emulated EEPROM region
 */
typedef struct {
          unsigned char  * ram;         //!< the base address of the RAM mirror
    const unsigned char  * rom_base;    //!< the base address of the pages in the FLASH
          unsigned short   size;        //!< the size of one page
          unsigned char    rom_pages;   //!< how many pages should be stored in the FLASH
          unsigned char    flags;       //!< internal state
} SB_Epp_Region;

#define SB_EEP_RAM_VALID   0x01

/**
 * Initializes the EEPROM region.
 *
 * If a valid page can be found inside the FLASH the data will be copied into
 * the RAM mirror.
 *
 * @param region        handle to the EEPROM reagion
 */
int sb_eep_init   (SB_Epp_Region * region);

/**
 * Write the current content of the RAM mirror of the EEPROM region into
 * a new page insinde the FLASH.
 * @param region        handle to the EEPROM reagion
 * @return              status of the operation (0 == OK)
 */
int sb_eep_update (SB_Epp_Region * region);

#endif /* SB_EEP_EMU_H_ */
