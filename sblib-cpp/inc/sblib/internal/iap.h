/*
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef sblib_iap_h
#define sblib_iap_h

#ifdef __cplusplus
extern "C" {
#endif

#include <sblib/types.h>


/**
 * Status code of IAP commands
 */
enum IAP_Status
{
    IAP_SUCCESS,                                //!< CMD_SUCCESS
    IAP_INVALID_COMMAND,                        //!< INVALID_COMMAND
    IAP_SRC_ADDR_ERROR,                         //!< SRC_ADDR_ERROR
    IAP_DST_ADDR_ERROR,                         //!< DST_ADDR_ERROR
    IAP_SRC_ADDR_NOT_MAPPED,                    //!< SRC_ADDR_NOT_MAPPED
    IAP_DST_ADDR_NOT_MAPPED,                    //!< DST_ADDR_NOT_MAPPED
    IAP_COUNT_ERROR,                            //!< COUNT_ERROR
    IAP_INVALID_SECTOR,                         //!< INVALID_SECTOR
    IAP_SECTOR_NOT_BLANK,                       //!< SECTOR_NOT_BLANK
    IAP_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION,//!< SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION
    IAP_COMPARE_ERROR,                          //!< COMPARE_ERROR
    IAP_BUSY                                    //!< BUSY
};


#ifndef IAP_EMULATION
  extern unsigned int __vectors_start__;
  extern unsigned int __top_MFlash32;
# define SB_FLASH_BASE_ADDRESS   ((unsigned char*) &__vectors_start__)
# define SB_FLASH_TOP_ADDRESS    ((unsigned char*) &__top_MFlash32)
#else
  /* for the test we simulate a 32k FlASH */
  extern unsigned char FLASH [];
# define SB_FLASH_BASE_ADDRESS   ((unsigned char *) FLASH)
# define SB_FLASH_TOP_ADDRESS    ((unsigned char *) FLASH + 0x1000*8)
#endif

#define SB_EEPROM_SECTOR_SIZE           0x1000
#define SB_EEPROM_FLASH_SECTOR_ADDRESS  (SB_FLASH_TOP_ADDRESS - SB_EEPROM_SECTOR_SIZE)

/**
 * Get the index of the FLASH sector for the passed address.
 *
 * @param address - the address inside the FLASH
 * @return The sector index of the address.
 */
int iapSectorOfAddress(const byte* address);

/**
 * Erase the specified sector.
 *  @param sector       index of the sector which should be erased
 *  @return             status code (0 == OK)
 */
IAP_Status iapEraseSector(int sector);

/**
 * Programs the specified number of bytes from the RAM to the specified location
 * inside the FLASH.
 * @param rom           start address of inside the FLASH
 * @param ram           start address if the buffer
 * @param size          number of bytes ot program
 * @return              status code, see enum IAP_Status above
 */
IAP_Status iapProgram(byte* rom, const byte* ram, unsigned int size);

/**
 * Read the unique ID of the CPU. The ID is 16 bytes long.
 *
 * @param uid - will contain the 16 byte UID after the call.
 *
 * @return Status code, see enum IAP_Status above
 */
IAP_Status iapReadUID(byte* uid);

/**
 * Read the 32 bit part identification number of the CPU.
 *
 * @param partId - will contain the 32 bit part identification number after the call.
 *
 * @return Status code, see enum IAP_Status above
 */
IAP_Status iapReadPartID(unsigned int* partId);

#ifdef __cplusplus
}
#endif

#endif /* SB_IAP_H_ */
