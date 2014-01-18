/*
 *  com_objects.cpp - EIB Communication objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/com_objects.h>

#include <sblib/eib/addr_tables.h>
#include <sblib/eib/apci.h>
#include <sblib/eib/user_memory.h>

#include <string.h>


// The COMFLAG_UPDATE flag, moved to the high nibble
#define COMFLAG_UPDATE_HIGH (COMFLAG_UPDATE << 4)

// The COMFLAG_TRANS_MASK mask, moved to the high nibble
#define COMFLAG_TRANS_MASK_HIGH (COMFLAG_TRANS_MASK << 4)


// The size of the object types BYTE_2 .. VARDATA in bytes
const byte objectTypeSizes[8] = { 2, 3, 4, 6, 8, 10, 14, 15 };


int objectSize(int objno)
{
    int type = objectType(objno);
    if (type < BYTE_2) return 1;
    return objectTypeSizes[type - BYTE_2];
}

/*
 * @return The number of communication objects.
 */
inline int objectCount()
{
    // The first byte of the config table contains the number of com-objects
    return *objectConfigTable();
}

/*
 * Find the first group address for the communication object. This is the
 * address that is used when sending the communication object.
 *
 * @param objno - the ID of the communication object
 * @return The group address, or 0 if none found.
 */
int objectSendAddr(int objno)
{
    byte* assocTab = assocTable();
    byte* assocTabEnd = assocTab + (*assocTab << 1);

    for (++assocTab; assocTab != assocTabEnd; assocTab += 2)
    {
        if (assocTab[1] == objno)
        {
            byte* addr = userEeprom.addrTab + (assocTab[0] << 1);
            return (addr[0] << 8) | addr[1];
        }
    }

    return 0;
}

/*
 * Create and send a group telegram.
 *
 * @param objno - the ID of the communication object
 * @param addr - the destination group address
 * @param isResponse - true if response telegram, false if write telegram
 */
void sendGroupTelegram(int objno, int addr, bool isResponse)
{
    byte* valuePtr = objectReadPtr(objno);
    int sz = objectSize(objno);

    bcu.sendTelegram[0] = 0xbc; // Control byte
    // 1+2 contain the sender address, which is set by sb_send_tel()
    bcu.sendTelegram[3] = addr >> 8;
    bcu.sendTelegram[4] = addr;
    bcu.sendTelegram[5] = 0xe0 | ((sz + 1) & 15);
    bcu.sendTelegram[6] = 0;
    bcu.sendTelegram[7] = isResponse ? 0x40 : 0x80;

    if (sz) memcpy(bcu.sendTelegram + 8, valuePtr, sz);
    else bcu.sendTelegram[7] |= *valuePtr & 0x3f;

    bus.sendTelegram(bcu.sendTelegram, 8 + sz);
}

void sendNextGroupTelegram()
{
    static int startIdx = 0;

    const ComConfig* configTab = &objectConfig(0);
    byte* flagsTab = objectFlagsTable();
    int flags, objno, numObjs = objectCount();

    for (objno = startIdx; objno < numObjs; ++objno)
    {
        flags = flagsTab[objno >> 1];
        if (objno & 1) flags >>= 4;

        if ((flags & COMFLAG_TRANS_MASK) == COMFLAG_TRANS_MASK)
        {
            flagsTab[objno >> 1] &= (objno & 1) ? 0x0f : 0xf0;

            if ((configTab[objno].config & COMCONF_TRANS_COMM) != COMCONF_TRANS_COMM)
                continue;

            int addr = objectSendAddr(objno);
            if (addr)
            {
                sendGroupTelegram(objno, addr, flags & COMFLAG_DATAREQ);
                startIdx = objno + 1;
                return;
            }
        }
    }

    startIdx = 0;
}

int nextUpdatedObject()
{
    static int startIdx = 0;

    byte* flagsTab = objectFlagsTable();
    int flags, objno, numObjs = objectCount();

    for (objno = startIdx; objno < numObjs; ++objno)
    {
        flags = flagsTab[objno >> 1];

        if (objno & 1) flags &= COMFLAG_UPDATE_HIGH;
        else flags &= COMFLAG_UPDATE;

        if (flags)
        {
            flagsTab[objno >> 1] &= ~flags;
            startIdx = objno + 1;
            return objno;
        }
    }

    startIdx = 0;
    return -1;
}

byte* objectReadPtr(int objno)
{
    // The object configuration
    const ComConfig& cfg = objectConfig(objno);

    if (cfg.config & COMCONF_VALUE_TYPE) // 0 if user RAM, >0 if user EEPROM
        return userEepromData + cfg.dataPtr;
    return userRamData + cfg.dataPtr;
}

void setObjectFlags(int objno, int flags)
{
    byte* flagsTab = objectFlagsTable();

    if (objno & 1)
        flags <<= 4;

    flagsTab[objno >> 1] |= flags;
}

void processGroupReadTelegram(int objno)
{
    setObjectFlags(objno, COMFLAG_TRANSREQ | COMFLAG_DATAREQ);
}

void processGroupWriteTelegram(int objno)
{
    byte* valuePtr = objectReadPtr(objno);
    int count = objectSize(objno);

    if (count > 0) memcpy(valuePtr, bus.telegram + 7, count);
    else *valuePtr = bus.telegram[6] & 0x3f;

    setObjectFlags(objno, COMFLAG_UPDATE);
}

void processGroupTelegram(int addr, int apci)
{
    const ComConfig* configTab = (const ComConfig*) (objectConfigTable() + 2);
    const byte* assocTab = assocTable();
    const int numAssoc = *assocTab;
    int objno, objConf;

    // Convert the group address into the index into the group address table
    const int gapos = indexOfAddr(addr);
    if (gapos < 0) return;

    // Loop over all entries in the association table, as one group address
    // could be assigned to multiple com-objects.
    for (int idx = 1; idx < numAssoc; idx += 2)
    {
        // Check if grp-address index in assoc table matches the dest grp address index
        if (gapos == assocTab[idx]) // We found an association for our addr
        {
            objno = assocTab[idx + 1];  // Get the com-object number from the assoc table
            objConf = configTab[objno].config;

            if (apci == APCI_GROUP_VALUE_WRITE_PDU)
            {
                // Check if communication and write are enabled
                if ((objConf & COMCONF_WRITE_COMM) == COMCONF_WRITE_COMM)
                    processGroupWriteTelegram(objno);
                break;
            }
            else if (apci == APCI_GROUP_VALUE_READ_PDU)
            {
                // Check if communication and read are enabled
                if ((objConf & COMCONF_READ_COMM) == COMCONF_READ_COMM)
                    processGroupReadTelegram(objno);
            }
        }
    }
}

byte* objectConfigTable()
{
#if BCU_TYPE == 0x10
    return userEepromData + userEeprom.commsTabPtr;
#else
#   error Unsupported BCU_TYPE
#endif
}

byte* objectFlagsTable()
{
#if BCU_TYPE == 0x10
    return userRamData + userEepromData[userEeprom.commsTabPtr + 1];
#else
#   error Unsupported BCU_TYPE
#endif
}
