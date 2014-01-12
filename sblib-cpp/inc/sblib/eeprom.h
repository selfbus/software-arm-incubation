/*
 *  eeprom.h - EEPROM emulation functions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_eeprom_h
#define sblib_eeprom_h

#include <sblib/platform.h>
#include <sblib/types.h>

#ifdef TO_BE_DONE

/**
 * This class emulates an EEPROM. The EEPROM values are kept in RAM and can be written
 * to FLASH when they have changed.
 *
 * The EEPROM shall be powers of 2 large, e.g. 256 or 1024. The last byte of the EEPROM
 * is used for internal stuff by the emulator. That means of a 256 byte area you are
 * only allowed to use the first 255 bytes.
 *
 * It is recommended to use a FLASH area that can contain more than one version of the
 * emulated EEPROM. The size of FLASH pages for most ARMs is 4k.
 *
 * Example:
 *
 * byte eepromData[1024];
 * Eeprom eeprom(1024, eepromData,
 */
class Eeprom
{
public:
    /**
     * Create an EEPROM emulator.
     *
     * @param size - The size of the EEPROM to emulate. The RAM array must have the same size.
     *               Make the size powers of 2 large (e.g. 256 or 1024).
     * @param ram - The RAM array that shall contain the temporary EEPROM values. This
     *              area must be large enough.
     * @param flash - The base address of the FLASH segment that will contain the EEPROM data.
     * @param pages - The number of EEPROM pages that are used. The total FLASH memory consumption
     *                is pages*size bytes.
     */
    Eeprom(byte* ram, unsigned short size);

    /**
     * Get a byte from the EEPROM.
     *
     * @return index - the number of the byte to get.
     * @return The byte from the EEPROM.
     */
    byte& operator[](short index);

    /**
     * Mark the EEPROM as being changed. Call this when the EEPROM values have been
     * modified to
     */
    void modified();

    /**
     * Test if the EEPROM values have been modified.
     */
    boolean isModified() const;

    /**
     * Write the EEPROM contents to flash. Call this method when the EEPROM was changed
     * to make the changes persistent. This method takes some time to complete.
     * The writing only happens when the EEPROM is marked as modified().
     *
     * Do not call flush() too often as FLASH storage can only be written aproximately
     * 10.000 times.
     */
    void flush();

    /**
     * Clear the EEPROM with zero bytes and mark it as dirty.
     */
    void clear();

protected:
    byte* ram;           //!< The RAM mirror that contains the current/modified EEPROM data
    unsigned short size; //!< The size of the EEPROM
    byte numPages;       //!< How many pages should be stored in the FLASH
    byte version;        //!< A layout version ID
    byte state;          //!< Internal state
};


//
//  Inline functions
//

ALWAYS_INLINE byte& operator[](short index)
{
    return ram[index];
}

ALWAYS_INLINE void modified()
{
    state = 1;
}

ALWAYS_INLINE boolean isModified() const
{
    return state == 1;
}

#endif /*TO_BE_DONE*/

#endif /*sblib_eeprom_h*/
