/*
 *  bcu.h - BCU specific stuff.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_bcu_h
#define sblib_bcu_h

#include <sblib/types.h>

// Fallback to BCU1 if BCU_TYPE is not defined
#ifndef BCU_TYPE
# define BCU_TYPE 0x10
#endif

// Include BCU specific stuff
#if BCU_TYPE == 0x10
#  include <sblib/eib/bcu_1.h>
#elif BCU_TYPE == 0x20
#  include <sblib/eib/bcu_2.h>
#else
#  error "Unsupported BCU type"
#endif

class UserRam;
class UserEeprom;


/**
 * The user RAM as class.
 */
extern UserRam userRam;

/**
 * The user RAM as data array.
 * The array is 256 bytes.
 */
extern byte* userRamData;

/**
 * The user EEPROM as class.
 */
extern UserEeprom userEeprom;

/**
 * The user EEPROM as data array.
 * The array is 255 bytes for BCU 1 and 1023 bytes for BCU 2.
 */
extern byte* userEepromData;


/**
 * The user RAM.
 */
class UserRam
{
public:
    byte reserved;       //!< 0x0000: Let's avoid address 0
    byte progRunning;    //!< 0x0001: Application program active (Selfbus extension)
    byte data1[0x5e];
    byte status;         //!< 0x0060: System status. See defines like SB_STATUS_PROG above
    byte data2[USER_RAM_SIZE - 0x60];
};


/**
 * The user EEPROM.
 */
class UserEeprom
{
public:
    byte optionReg;      //!< 0x0100: EEPROM option register
    byte manuDataH;      //!< 0x0101: Manufacturing data high byte
    byte manuDataL;      //!< 0x0102: Manufacturing data low byte
    byte manufacturerH;  //!< 0x0103: Software manufacturer high byte
    byte manufacturerL;  //!< 0x0104: Software manufacturer low byte
    byte deviceTypeH;    //!< 0x0105: Device type high byte
    byte deviceTypeL;    //!< 0x0106: Device type low byte
    byte version;        //!< 0x0107: Software version
    byte checkLimit;     //!< 0x0108: EEPROM check limit
    byte peiTypeExpectd; //!< 0x0109: PEI type that the software requires
    byte syncRate;       //!< 0x010a: Baud rate for serial synchronous PEI
    byte portCDDR;       //!< 0x010b: Port C DDR settings (PEI type 17)
    byte portADDR;       //!< 0x010c: Port A DDR settings
    byte runError;       //!< 0x010d: Runtime error flags
    byte routeCnt;       //!< 0x010e: Routing count constant
    byte maxRetransmit;  //!< 0x010f: INAK and BUSY retransmit limit
    byte confDesc;       //!< 0x0110: Configuration descriptor
    byte assocTabPtr;    //!< 0x0111: Low byte of the pointer to association table (BCU1 only)
    byte commsTabPtr;    //!< 0x0112: Low byte of the pointer to communication objects table (BCU1 only)
    byte usrInitPtr;     //!< 0x0113: Low byte of the pointer to user initialization function (BCU1 only)
    byte usrProgPtr;     //!< 0x0114: Low byte of the pointer to user program function (BCU1 only)
#if BCU_TYPE == 0x10
    byte usrSavePtr;     //!< 0x0115: Low byte of the pointer to user save function (BCU1 only)
    byte addrTabSize;    //!< 0x0116: Size of the address table
    byte addrTab[2];     //!< 0x0117+: Address table, 2 bytes per entry. Real array size is addrTabSize*2
    byte user[220];      //!< 0x0116: User EEPROM: 220 bytes (BCU1)
    byte checksum;       //!< 0x01ff: EEPROM checksum (BCU1 only)
#elif BCU_TYPE == 0x20
    byte appType;        //!< 0x0115: Application program type: 0=BCU2, else BCU1
    byte addrTabSize;    //!< 0x0116: Size of the address table ?
    byte addrTab[2];     //!< 0x0117+:Address table, 2 bytes per entry. Real array size is addrTabSize*2
    byte user[856];      //!< 0x0119+:User EEPROM: 856 bytes (BCU2)
                         //!< ------  System EEPROM below
    byte appLoaded;      //!< 0x0470: Application load control state (BCU2, Selfbus extension)
    byte appRunning;     //!< 0x0471: Application run control state (BCU2, Selfbus extension)
    byte addrTabLoaded;  //!< 0x0472: Address table load control state (BCU2, Selfbus extension)
    byte assocTabLoaded; //!< 0x0473: Association table load control state (BCU2, Selfbus extension)
    word serviceControl; //!< 0x0474: Service control (BCU2, Selfbus extension)
    byte system[137];    //!< 0x0476: Rest of the system EEPROM (BCU2 only)
#endif /*BCU_TYPE*/
};


/** End address of the user RAM +1, when ETS talks with us. */
#define USER_RAM_END (USER_RAM_START + USER_RAM_SIZE)

/** End address of the user EEPROM +1, when ETS talks with us. */
#define USER_EEPROM_END (USER_EEPROM_START + USER_EEPROM_SIZE)

#endif /*sblib_bcu_h*/
