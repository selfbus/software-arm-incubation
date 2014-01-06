/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sb_memory_h
#define sb_memory_h

/*
 * This file contains the structures and variables for the user RAM and EEPROM
 * for the BCU emulation.
 */

// Fallback definition if no BCU type is set
#if !defined(SB_BCU1) && !defined(SB_BCU2)
#   define SB_BCU1
#endif


/*
 * Defines for the system status byte SbUserRam.status (usually at userram[60]).
 * See BCU1 help for detailed description.
 */

// System status: programming mode: 0=normal mode, 1=programming mode
#define SB_STATUS_PROG    0x01

// System status: link layer mode
#define SB_STATUS_LL      0x02

// System status; transport layer enabled
#define SB_STATUS_TL      0x04

// System status: application layer enabled
#define SB_STATUS_AL      0x08

// System status: serial PEI enabled
#define SB_STATUS_SER     0x10

// System status: application program enabled
#define SB_STATUS_USR     0x20

// System status: download mode enabled
#define SB_STATUS_DWN     0x40

// System status: parity bit
#define SB_STATUS_PARITY  0x80


/*
 * User RAM and EEPROM structures for BCU1.
 * See BCU1 help.
 */
#if defined(SB_BCU1)

# define SB_USERRAM_START 0
# define SB_USERRAM_SIZE 256

# define SB_EEPROM_START 0x100
# define SB_EEPROM_SIZE 256

#elif defined(SB_BCU2)

# define SB_USERRAM_START 0
# define SB_USERRAM_SIZE 256

# define SB_EEPROM_START 0x100
# define SB_EEPROM_SIZE 1024

#endif /*SB_BCU2*/


/*
 * User RAM and EEPROM structures for BCU2
 */
typedef struct SbUserRam
{
    unsigned char reserved;       // 0x0000: Let's avoid address 0
    unsigned char progRunning;    // 0x0001: Application program active (Selfbus extension)
    unsigned char data1[0x5e];
    unsigned char status;         // 0x0060: System status. See defines like SB_STATUS_PROG above
    unsigned char data2[SB_USERRAM_SIZE - 0x60];
} SbUserRam;

/** End addres of userram +1 */
#define SB_USERRAM_END (SB_EEPROM_START + SB_EEPROM_SIZE)

/** End address of eeprom +1 */
#define SB_EEPROM_END (SB_EEPROM_START + SB_EEPROM_SIZE)


#if defined(SB_BCU1) || defined(SB_BCU2)
typedef struct SbEeprom
{
    unsigned char optionReg;      // 0x0100: EEPROM option register
    unsigned char manuDataH;      // 0x0101: Manufacturing data high byte
    unsigned char manuDataL;      // 0x0102: Manufacturing data low byte
    unsigned char manufacturerH;  // 0x0103: Software manufacturer high byte
    unsigned char manufacturerL;  // 0x0104: Software manufacturer low byte
    unsigned char deviceTypeH;    // 0x0105: Device type high byte
    unsigned char deviceTypeL;    // 0x0106: Device type low byte
    unsigned char version;        // 0x0107: Software version
    unsigned char checkLimit;     // 0x0108: EEPROM check limit
    unsigned char peiTypeExpectd; // 0x0109: PEI type that the software requires
    unsigned char syncRate;       // 0x010a: Baud rate for serial synchronous PEI
    unsigned char portCDDR;       // 0x010b: Port C DDR settings (PEI type 17)
    unsigned char portADDR;       // 0x010c: Port A DDR settings
    unsigned char runError;       // 0x010d: Runtime error flags
    unsigned char routeCnt;       // 0x010e: Routing count constant
    unsigned char maxRetransmit;  // 0x010f: INAK and BUSY retransmit limit
    unsigned char confDesc;       // 0x0110: Configuration descriptor
    unsigned char assocTabPtr;    // 0x0111: Low byte of the pointer to association table (BCU1 only)
    unsigned char commsTabPtr;    // 0x0112: Low byte of the pointer to communication objects table (BCU1 only)
    unsigned char usrInitPtr;     // 0x0113: Low byte of the pointer to user initialization function (BCU1 only)
    unsigned char usrProgPtr;     // 0x0114: Low byte of the pointer to user program function (BCU1 only)
#if defined(SB_BCU1)
    unsigned char usrSavePtr;     // 0x0115: Low byte of the pointer to user save function (BCU1 only)
    unsigned char addrTabSize;    // 0x0116: Size of the address table
    unsigned char addrTab[2];     // 0x0117+: Address table, 2 bytes per entry. Real array size is addrTabSize*2
    unsigned char user[220];      // 0x0116: User EEPROM: 220 bytes (BCU1)
    unsigned char checksum;       // 0x01ff: EEPROM checksum (BCU1 only)
#else //defined(SB_BCU2)
    unsigned char appType;        // 0x0115: Application program type: 0=BCU2, else BCU1
    unsigned char addrTabSize;    // 0x0116: Size of the address table ?
    unsigned char addrTab[2];     // 0x0117+:Address table, 2 bytes per entry. Real array size is addrTabSize*2
    unsigned char user[856];      // >0x0119:User EEPROM: 856 bytes (BCU2)
                                  // ------  System EEPROM below
    unsigned char appLoaded;      // 0x0470: Application load control state (BCU2, Selfbus extension)
    unsigned char appRunning;     // 0x0471: Application run control state (BCU2, Selfbus extension)
    unsigned char addrTabLoaded;  // 0x0472: Address table load control state (BCU2, Selfbus extension)
    unsigned char assocTabLoaded; // 0x0473: Association table load control state (BCU2, Selfbus extension)
    unsigned char system[139];    // 0x0474: Rest of the system EEPROM (BCU2 only)
#endif
} SbEeprom;
#endif


/**
 * Communication object descriptor.
 *
 * @see SbComsTab - this structure is used in the communications table.
 */
typedef struct
{
    /** Data pointer. Either to userRam or to eeprom, depending on bit 5 of the config byte */
    unsigned char dataPtr;

    /** Configuration. See SB_COMDESC_xy defines below */
    unsigned char config;

    /** Data type. See SB_COMTYPE_xy defines below */
    unsigned char type;
} SbComDesc;


/** Communication object configuration: transmission priority */
#define SB_COMCONF_PRIO_MASK 0x03

/** Communication object configuration: communication enabled */
#define SB_COMCONF_COMM 0x04

/** Communication object configuration: read enabled */
#define SB_COMCONF_READ 0x08

/** Communication object configuration: write enabled */
#define SB_COMCONF_WRITE 0x10

/** Communication object configuration: memory segment: 0=userRam, 1=eeprom */
#define SB_COMCONF_MEM 0x20

/** Communication object configuration: transmit enabled */
#define SB_COMCONF_TRANS 0x40

/** Communication object configuration: reserved, must be 1 */
#define SB_COMCONF_RES 0x80


/** Communication object type: 1 bit */
#define SB_COMTYPE_1BIT 0

/** Communication object type: 2 bit */
#define SB_COMTYPE_2BIT 1

/** Communication object type: 3 bit */
#define SB_COMTYPE_3BIT 2

/** Communication object type: 4 bit */
#define SB_COMTYPE_4BIT 3

/** Communication object type: 5 bit */
#define SB_COMTYPE_5BIT 4

/** Communication object type: 6 bit */
#define SB_COMTYPE_6BIT 5

/** Communication object type: 7 bit */
#define SB_COMTYPE_7BIT 6

/** Communication object type: 1 byte */
#define SB_COMTYPE_1BYTE 7

/** Communication object type: 2 bytes */
#define SB_COMTYPE_2BYTE 8

/** Communication object type: 3 bytes */
#define SB_COMTYPE_3BYTE 9

/** Communication object type: float */
#define SB_COMTYPE_FLOAT 10

/** Communication object type: 6 bytes */
#define SB_COMTYPE_DATA6 11

/** Communication object type: double */
#define SB_COMTYPE_DOUBLE 12

/** Communication object type: 10 bytes */
#define SB_COMTYPE_DATA10 13

/** Communication object type: 14 bytes */
#define SB_COMTYPE_MAXDATA 14

/** Communication object type: 1-14 bytes */
#define SB_COMTYPE_VARDATA 15


/**
 * Communications table.
 */
typedef struct
{
    /** Number of objects */
    unsigned char count;

    /** UserRAM pointer to flag-table. */
    unsigned char flagTablePtr;

    /** Table of object descriptors. */
    SbComDesc desc[];
} SbComTab;


/**
 * Structure for the global variables.
 */
typedef struct
{
    /** Is the application running? */
    unsigned char appRunning;
} SbGlobal;

/**
 * The global variables.
 */
extern SbGlobal* sbGlobal;


/**
 * User RAM, structured. Points to the same memory as sbUserRamData[]
 */
extern SbUserRam *sbUserRam;

/**
 * User RAM, plain data array. Points to the same memory as sbUserRam
 */
extern unsigned char sbUserRamData[SB_USERRAM_SIZE];


/**
 * User EEPROM, structured. Points to the same memory as sbEepromData[]
 */
extern SbEeprom *sbEeprom;

/**
 * User EEPROM, plain data array. Points to the same memory as sbEeprom
 */
extern unsigned char* sbEepromData;


#endif /*sb_memory_h*/
