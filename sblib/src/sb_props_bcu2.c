/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifdef SB_BCU2

#include "sb_props.h"

#include "sb_memory.h"
#include "sb_bus.h"
#include "sb_utils.h"

#include <string.h>


// Documentation:
// see KNX 6/6 Profiles, p. 94+
// see KNX 3/7/3 Standardized Identifier Tables, p. 11+


//
// Constant property values
//
typedef struct
{
    unsigned char serial[6];
    unsigned char order[10];
} SbConstPropValues;

static const SbConstPropValues sbConstPropValues =
{
    /*serial number*/ { 0x11, 0x22, 0x33, 0x44, 0x55, 0x67 },
    /*order number*/  { 0x11, 0x12, 0x00, 0x00, 0x22, 0x23, 0x00, 0x00, 0x33, 0x34 }
};


//
// The properties of the device object
//
static const SbPropDef sbDeviceObjectProps[] =
{
    // Serial number: 6 byte data, stored in sbConstPropValues
    { SB_PID_SERIAL_NUMBER, SB_PDT_GENERIC_06|SB_PMEM_CONST, SB_OFFSET_OF(SbConstPropValues, serial) },

    // Manufacturer ID: unsigned int, stored in user EEPROM
    { SB_PID_MANUFACTURER_ID, SB_PDT_UNSIGNED_INT|SB_PMEM_EEPROM, SB_OFFSET_OF(SbEeprom, manufacturerH) },

    // Order number: 10 byte data, stored in sbConstPropValues
    { SB_PID_ORDER_INFO, SB_PDT_GENERIC_10|SB_PMEM_CONST, SB_OFFSET_OF(SbConstPropValues, order) },

    // PEI type: 1 byte, stored here
    { SB_PID_PEI_TYPE, SB_PDT_UNSIGNED_CHAR|SB_PMEM_DIRECT, 0x02 },

    // End of table
    SB_PROPDEF_END
};

//
// The properties of the address table object
//
static const SbPropDef sbAddrTabObjectProps[] =
{
    { SB_PID_LOAD_STATE_CONTROL, SB_PDT_CONTROL|SB_PMEM_EEPROM, SB_OFFSET_OF(SbEeprom, addrTabLoaded) },
    SB_PROPDEF_END
};

//
// The properties of the association table object
//
static const SbPropDef sbAssocTabObjectProps[] =
{
    { SB_PID_LOAD_STATE_CONTROL, SB_PDT_CONTROL|SB_PMEM_EEPROM, SB_OFFSET_OF(SbEeprom, assocTabLoaded) },
    SB_PROPDEF_END
};

//
// The properties of the application program object
//
static const SbPropDef sbAppObjectProps[] =
{
    { SB_PID_LOAD_STATE_CONTROL, SB_PDT_CONTROL|SB_PMEM_EEPROM, SB_OFFSET_OF(SbEeprom, appLoaded) },
    { SB_PID_RUN_STATE_CONTROL, SB_PDT_CONTROL|SB_PMEM_GLOBAL, SB_OFFSET_OF(SbGlobal, appRunning) },
    { SB_PID_PROG_VERSION, SB_PDT_GENERIC_05|SB_PMEM_EEPROM, SB_OFFSET_OF(SbEeprom, manufacturerH) },
    SB_PROPDEF_END
};


#define SB_PROP_OBJS_SIZE 4

//
// The interface objects
//
static const SbPropDef* sbPropObjsTab[SB_PROP_OBJS_SIZE] =
{
    sbDeviceObjectProps,    // Object 0
    sbAddrTabObjectProps,   // Object 1
    sbAssocTabObjectProps,  // Object 2
    sbAppObjectProps        // Object 3
};


/**
 * Get the property definition of an object's property.
 *
 * @param objectIdx - the object index
 * @param propertyId - the property ID
 * @return The property definition, or 0 if not found.
 */
const SbPropDef* sb_get_prop_def(unsigned char objectIdx, unsigned char propertyId)
{
    const SbPropDef* props;

    if (objectIdx < SB_PROP_OBJS_SIZE)
    {
        for (props = sbPropObjsTab[objectIdx]; props->pid; ++props)
        {
            if (props->pid == propertyId)
                return props;
        }
    }

    return 0;
}

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
                                unsigned char count, unsigned short start)
{
    const SbPropDef* def = sb_get_prop_def(objectIdx, propertyId);
    if (!def) return 0;

    unsigned char type = def->type & SB_PDT_MASK;
    unsigned short len = count * type;
    unsigned char* valuePtr = 0;

    switch (def->type & SB_PMEM_MASK)
    {
    case SB_PMEM_DIRECT:
        valuePtr = (unsigned char*) &def->valIdx;
        break;

    case SB_PMEM_EEPROM:
        valuePtr = sbEepromData + def->valIdx;
        break;

    case SB_PMEM_CONST:
        valuePtr = ((unsigned char*) &sbConstPropValues) + def->valIdx;
        break;

    case SB_PMEM_GLOBAL:
        valuePtr = ((unsigned char*) sbGlobal) + def->valIdx;
        break;

    default:
#ifdef DEBUG
        while (1) {}  // error: stop here
#endif
        return 0;
    }

#ifdef DEBUG
    if (!valuePtr)
        while (1) {}  // error: stop here
#endif

    --start;
    memcpy(sbSendTelegram + 12, valuePtr + start * type, len);
    sbSendTelegram[5] += len;

    return 1;
}

#endif //SB_BCU2
