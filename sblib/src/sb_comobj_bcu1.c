/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *                     Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "sb_memory.h"

#ifdef SB_BCU1

#include "sb_comobj.h"
#include "sb_const.h"
#include "sb_bus.h"

#include <string.h>

#define SEND_TYPE_RESPONSE 0x40
#define SEND_TYPE_WRITE 0x80

// Mask for clearing the relevant com flags when sending a com-object at the low nibble
#define SB_COMFLAG_CLEAR_LOW (~(SB_COMFLAG_TRANS_MASK | SB_COMFLAG_DATAREQ) | 0xf0)

// Mask for clearing the relevant com flags when sending a com-object at the high nibble
#define SB_COMFLAG_CLEAR_HIGH ((~(SB_COMFLAG_TRANS_MASK | SB_COMFLAG_DATAREQ) << 4) | 0x0f)


/**
 * Get the object flags.
 *
 * @param objno - the number of the com-object.
 * @return The object flags of the com-object.
 */
unsigned char sb_get_objflags(unsigned short objno)
{
    return sbEepromData[sbEeprom->commsTabPtr + 3 + 3 * objno];
}

/**
 * Get the communication (RAM) flags of a com-object.
 *
 * @param objno - the com-object to query
 * @return The communication flags.
 */
unsigned char sb_get_flags(unsigned short objno)
{
    // The flags table contains two com-objects per byte

    unsigned char flags = sbUserRamData[sbEepromData[sbEeprom->commsTabPtr + 1] + (objno >> 1)];
    if (objno & 1) flags >>= 4;

    return flags & 15;
}

/**
 * Set one or more communication (RAM) flags of a com-object. This does not
 * change already set flags of the com-object. See sb_unset_flags().
 * See SB_COMFLAG_ defines below.
 *
 * @param objno - the com-object to process
 * @param flags - the flags to set.
 */
void sb_set_flags(unsigned short objno, unsigned char flags)
{
    // The flags table contains two com-objects per byte

    if (objno & 1) flags <<= 4;
    else flags &= 15;

    sbUserRamData[sbEepromData[sbEeprom->commsTabPtr + 1] + (objno >> 1)] |= flags;
}

/**
 * Unset one or more communication (RAM) flags of a com-object. Only the specified
 * flags are cleared. See sb_set_flags().
 * See SB_COMFLAG_ defines below.
 *
 * @param objno - the com-object to process
 * @param flags - the flags to clear. Use SB_COMFLAG_ALL to clear all flags.
 */
void sb_unset_flags(unsigned short objno, unsigned char flags)
{
    // The flags table contains two com-objects per byte

    if (objno & 1) flags <<= 4;
    else flags &= 15;

    sbUserRamData[sbEepromData[sbEeprom->commsTabPtr + 1] + (objno >> 1)] &= ~flags;
}

/**
 * Schedule a com-object for sending. This will send a group telegram
 * with the com-object's value.
 *
 * A group telegram is only sent if a group address exists for this
 * com-object and communication and transmit flags are set for the com-object.
 *
 * @param objno - the number of the com object to send.
 */
void sb_send_obj_value(unsigned short objno)
{
    if ((sb_get_objflags(objno) & SB_COMOBJ_CONF_TRANS_COMM) == SB_COMOBJ_CONF_TRANS_COMM)
        sb_set_flags(objno, SB_COMFLAG_TRANSREQ);
}

/**
 * Get the pointer to the value of a com-object.
 *
 * @param objno - the com-object to query
 * @return The pointer to the com-object's value
 */
void* sb_get_value_ptr(unsigned short objno)
{
    // Pointer to the object descriptor
    SbComDesc* desc = (SbComDesc*) (sbEepromData + sbEeprom->commsTabPtr + objno * 3 + 2);

    if (desc->config & SB_COMCONF_MEM) // 0 if user RAM, >0 if user EEPROM
        return sbEepromData + desc->dataPtr;
    return sbUserRamData + desc->dataPtr;
}

/**
 * Find the first index of a group address in the address table.
 * The result will be >= 1 as the first entry of the address table is the
 * physical address of the device.
 *
 * @param groupAddr - the group address to find.
 * @return The index of the group address, 0 if not found.
 */
unsigned short sb_index_of_group_addr(unsigned short groupAddr)
{
    const unsigned char addrHigh = groupAddr >> 8;;
    const unsigned char addrLow = groupAddr;
    unsigned char i;

    if (sbEeprom->addrTabSize < 255) // it's 255 if it was never flashed
    {
        const unsigned char high = sbEeprom->addrTabSize << 1;
        for (i = 2; i <= high; i += 2)
        {
            if (sbEeprom->addrTab[i] == addrHigh &&
                sbEeprom->addrTab[i + 1] == addrLow)
            {
                return i >> 1;
            }
        }
    }

    return 0;
}

/**
 * Find the first group address for the given com-object that has write and
 * communication enabled.
 *
 * @param objno - the com-object to find
 * @return The group address, or 0 if not found.
 */
unsigned short sb_get_send_group_addr(unsigned short objno)
{
    unsigned char* assocTab = sbEepromData + sbEeprom->assocTabPtr;
    unsigned char* assocTabEnd = assocTab + (assocTab[0] << 1);

    for (++assocTab; assocTab != assocTabEnd; assocTab += 2)
    {
        if (assocTab[1] == objno)
        {
            unsigned char conIdx = assocTab[0] << 1;
            return (sbEeprom->addrTab[conIdx] << 8) | sbEeprom->addrTab[conIdx + 1];
        }
    }

    return 0;
}

#ifndef SB_CUSTOM_READWRITE_VALUE

/**
 * Called when a write-value-request group telegram was received.
 * The value to be written is stored in sbRecvTelegram[].
 *
 * @param objno - the com-object to write the value.
 */
void sb_write_value_req(unsigned short objno)
{
    unsigned char* valuePtr = sb_get_value_ptr(objno);
    short count = (sbRecvTelegram[5] & 15) - 1;

    if (count > 0) memcpy(valuePtr, sbRecvTelegram + 7, count);
    else *valuePtr = sbRecvTelegram[6] & 0x3f;

    sb_set_flags(objno, SB_COMFLAG_UPDATE);
}

/**
 * Called when a read-value-request group telegram was received.
 *
 * @param objno - the com-object to write the value.
 */
void sb_read_value_req(unsigned short objno)
{
    if ((sb_get_objflags(objno) & SB_COMOBJ_CONF_READ_COMM) == SB_COMOBJ_CONF_READ_COMM)
        sb_set_flags(objno, SB_COMFLAG_TRANSREQ | SB_COMFLAG_DATAREQ);
}

#endif // not SB_CUSTOM_READWRITE_VALUE

/**
 * Process a multicast group telegram.
 *
 * This function is called by sb_process_tel(). It is usually not required to call
 * this function from within a user program. The telegram is stored in sbRecvTelegram[].
 *
 * @param destAddr - the destination group address.
 */
void sb_process_group_tel(unsigned short destAddr, unsigned char apci)
{
    // The association table
    unsigned char* assocTab = sbEepromData + sbEeprom->assocTabPtr;

    // The maximum index in the association table
    const unsigned short assmax = assocTab[0] * SB_ASSOC_ENTRY_SIZE;

    unsigned short gapos;
    unsigned short objno;
    unsigned short objflags;
    unsigned short idx;

    // convert the group address into the index into the group address table
    gapos = sb_index_of_group_addr(destAddr);
    if (!gapos) return;

    // Loop over all entries, as one group address could be assigned to multiple com-objects
    for (idx = 1; idx < assmax; idx += 2)
    {
        // check of grp-address index in assoc table matches the dest grp address index
        if (gapos == assocTab[idx]) // we found a assoc for our destAddr
        {
            objno    = assocTab[idx + 1];      // get the com object number from the assoc table
            objflags = sb_get_objflags(objno); // get the flags for this com-object

            // check if it is a group write request
            if ((apci & 0xC0) == SB_WRITE_GROUP_REQUEST)
            {
                // check if
                // - communication is enabled (bit 2)
                // - write is enabled         (bit 4)
                if ((objflags & SB_COMOBJ_CONF_READ_COMM) == SB_COMOBJ_CONF_READ_COMM)
                    sb_write_value_req(objno);
            }
            else if (apci == SB_READ_GROUP_REQUEST)
            {
                // check if
                // - communication is enabled (bit 2)
                // - read  is enabled         (bit 3)
                if ((objflags & SB_COMOBJ_CONF_WRITE_COMM) == SB_COMOBJ_CONF_WRITE_COMM)
                    sb_read_value_req(objno);
                break;
            }
        }
    }
}

/**
 * Create and send a group telegram.
 *
 * @param objno - the com-object to send
 * @param destAddr - the destination group address
 * @param isResponse - 0 if write, not 0 if response telegram
 */
void sb_send_group_tel(unsigned short objno, unsigned short destAddr, unsigned char isResponse)
{
    // Pointer to the object descriptor
    SbComDesc* desc = (SbComDesc*) (sbEepromData + sbEeprom->commsTabPtr + objno * 3 + 2);
    unsigned char* valuePtr = sb_get_value_ptr(objno);

    short count = desc->type - 5;
    if (count < 0) count = 0;

    sbSendTelegram[0] = 0xbc; // Control byte
    // 1+2 contain the sender address, which is set by sb_send_tel()
    sbSendTelegram[3] = destAddr >> 8;
    sbSendTelegram[4] = destAddr;
    sbSendTelegram[5] = 0xe0 | ((count + 1) & 15);
    sbSendTelegram[6] = 0;
    sbSendTelegram[7] = isResponse ? 0x40 : 0x80;

    if (count) memcpy(sbSendTelegram + 8, valuePtr, count);
    else sbSendTelegram[7] |= *valuePtr & 0x3f;

    sb_send_tel(sbSendTelegram, 8 + count);
}

/**
 * Process all com-objects. For every com-object that has the given flags set,
 * call the callback function. The flags are cleared during processing.
 *
 * @param flags - the communication (RAM) flags that must be set.
 * @param callback - pointer to the function to call when the flags are set.
 *                   The function gets the com-object number as an argument
 *
 * @example sb_process_flags(SB_COMFLAG_UPDATE, &comobj_updated);
 */
void sb_process_flags(unsigned char flags, void (*callback)(unsigned char))
{
    unsigned char matchLow = flags;
    unsigned char matchHigh = flags << 4;

    unsigned char* commsTab = sbEepromData + sbEeprom->commsTabPtr;
    unsigned char* flagsTab = sbUserRamData + commsTab[1];
    unsigned char objFlags, objno, numObjs = commsTab[0];

    for (objno = 0; objno < numObjs; ++objno)
    {
        objFlags = flagsTab[objno >> 1];

        if (objno & 1)
        {
            if ((objFlags & matchHigh) == matchHigh)
            {
                flagsTab[objno >> 1] &= ~matchHigh;
                (*callback)(objno);
            }
        }
        else
        {
            if ((objFlags & matchLow) == matchLow)
            {
                flagsTab[objno >> 1] &= ~matchLow;
                (*callback)(objno);
            }
        }
    }
}

/**
 * Send the next group value write/response telegram. Does nothing if there is
 * no group value marked for sending.
 */
void sb_send_next_tel()
{
    static unsigned short startIdx = 0;

    unsigned char* commsTab = sbEepromData + sbEeprom->commsTabPtr;
    unsigned char* flagsTab = sbUserRamData + commsTab[1];
    unsigned short addr, objno, numObjs = commsTab[0];
    unsigned char flags;

    for (objno = startIdx; objno < numObjs; ++objno)
    {
        flags = flagsTab[objno >> 1];
        if (objno & 1) flags >>= 4;
        else flags &= 15;

        if ((flags & SB_COMFLAG_TRANS_MASK) == SB_COMFLAG_TRANSREQ)
        {
            flagsTab[objno >> 1] &= (objno & 1) ? SB_COMFLAG_CLEAR_HIGH : SB_COMFLAG_CLEAR_LOW;

            addr = sb_get_send_group_addr(objno);
            if (addr)
            {
                startIdx = objno + 1;
                sb_send_group_tel(objno, addr, flags & SB_COMFLAG_DATAREQ);
                return;
            }
        }
    }

    startIdx = 0;
}

#endif /*SB_BCU1*/
