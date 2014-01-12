/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sb_props_h
#define sb_props_h

#ifdef SB_BCU2

// See KNX 3/7/3 Standardized Identifier Tables, p. 17 for property data types

// Property data type: 1 byte load/run control
#define SB_PDT_CONTROL         0

// Property data type: 1 byte unsigned char
#define SB_PDT_UNSIGNED_CHAR   1

// Property data type: 2 bytes unsigned short integer
#define SB_PDT_UNSIGNED_INT    2

// Property data type: 4 bytes unsigned long integer
#define SB_PDT_UNSIGNED_LONG   4

// Property data type: 4 bytes generic
#define SB_PDT_GENERIC_04      4

// Property data type: 5 bytes generic
#define SB_PDT_GENERIC_05      5

// Property data type: 6 bytes generic
#define SB_PDT_GENERIC_06      6

// Property data type: 10 bytes generic
#define SB_PDT_GENERIC_10      10

// Property data type: 0-15 bytes string, zero terminated
#define SB_PDT_STRING          15

// Bitmask for SB_PDT_xx defines
#define SB_PDT_MASK            0x3f


// Property memory location:  constant value, stored in the valIdx field
// of the SbPropDef structure.
#define SB_PMEM_DIRECT         0x00

// Property memory location:  constant value, stored in sbConstValues[]
// at the index of the valIdx field of the SbPropDef structure.
#define SB_PMEM_CONST          0x40

// Property memory location:  variable value, stored in sbEepromData[]
// at the index of the valIdx field of the SbPropDef structure.
#define SB_PMEM_EEPROM         0x80

// Property memory location: variable value, stored in sbGlobal at the
// byte-offset of the valIdx field of the SbPropDef structure.
#define SB_PMEM_GLOBAL         0xc0

// Bitmask for SB_PMEM_xx defines
#define SB_PMEM_MASK           0xc0


/**
 * The defintion of an interface object property.
 */
typedef struct SbPropDef
{
    // The property ID
    unsigned char pid;

    // Bit 0..5: the property data type, see SB_PDT_xxx defines
    // Bit 6..7: memory location, see SB_PMEM_xxx defines
    unsigned char type;

    // Value index or value itself. Depends on bit 6..7 of the type field
    unsigned short valIdx;
} SbPropDef;

// Define that marks the end of a SbPropDef table
#define SB_PROPDEF_END { 0, 0, 0 }


// Device object property: service control
#define SB_PID_SERVICE_CONTROL     8

// Device object property: serial number
#define SB_PID_SERIAL_NUMBER       11

// Device object property: manufacturer ID
#define SB_PID_MANUFACTURER_ID     12

// Device object property: order info
#define SB_PID_ORDER_INFO          15

// Device object property: PEI type
#define SB_PID_PEI_TYPE            16

// Device object property: port configuration
#define SB_PID_PORT_CONFIGURATION  17


// Application object property: load state control
#define SB_PID_LOAD_STATE_CONTROL  5

// Application object property: run state control
#define SB_PID_RUN_STATE_CONTROL   6

// Application object property: program version
#define SB_PID_PROG_VERSION        13


#endif //SB_BCU2


/**
 * Process a property-value read telegram.
 *
 * @param objectIdx - the object index
 * @param propertyId - the property ID
 * @param count - the number of elements to read
 * @param start - the index of the first element to read
 *
 * @return 1 if the request was processed, 0 if objectIdx or propertyId are
 *         unsupported.
 */
unsigned char sb_props_read_tel(unsigned char objectIdx, unsigned char propertyId,
                                unsigned char count, unsigned short start);

/**
 * Process a property-value write telegram.
 * The data to be written is stored in sbRecvTelegram[12+].
 *
 * @param objectIdx - the object index
 * @param propertyId - the property ID
 * @param count - the number of elements to write
 * @param start - the index of the first element to write
 *
 * @return 1 if the request was processed, 0 if objectIdx or propertyId are
 *         unsupported or the property is not writable.
 */
unsigned char sb_props_write_tel(unsigned char objectIdx, unsigned char propertyId,
                                 unsigned char count, unsigned short start);

#endif /*sb_props_h*/
