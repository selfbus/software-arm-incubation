/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *                     Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sb_proto.h"
#include "sb_bus.h"
#include "sb_const.h"
#include "sb_eeprom.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif


#define SB_USERRAM_SIZE 256
unsigned char userram[SB_USERRAM_SIZE];

// Ring buffer for send requests.
unsigned int sbSendRing[SB_SEND_RING_SIZE];

// Index in sbSendRing[] where the next write will occur.
unsigned short sbSendRingWrite;

// Index in sbSendRing[] where the next read will occur.
unsigned short sbSendRingRead;

// Physical address of the remote device when a direct data connection is active.
// Zero if no connection is active.
unsigned short sbConnectedAddr;

// Sequence number for sending telegrams
unsigned char sbConnectedSeqNo;

// The sequence number shall be incremented upon successful send
unsigned char sbIncConnectedSeqNo;

// for memory reads, holds the number of requested bytes
unsigned char sbMemReadNoBytes;

// for memory reads, holds the start address of the request
unsigned char sbMemReadAddress;

// Com object configuration flag: transmit + communication enabled
#define SB_COMOBJ_CONF_TRANS_COMM (SB_COMOBJ_CONF_COMM | SB_COMOBJ_CONF_TRANS)

// Com object configuration flag: read + communication enabled
#define SB_COMOBJ_CONF_READ_COMM (SB_COMOBJ_CONF_COMM | SB_COMOBJ_CONF_READ)

// Com object configuration flag: write + communication enabled
#define SB_COMOBJ_CONF_WRITE_COMM (SB_COMOBJ_CONF_COMM | SB_COMOBJ_CONF_WRITE)

static void sb_update_memory(signed char count, unsigned short address, unsigned char * data);
static unsigned short sb_grp_address2index(unsigned short address);


/**
 * Process a unicast telegram with our physical address as destination address.
 * The telegram is stored in sbRecvTelegram[].
 *
 * When this function is called, the sender address is != 0 (not a broadcast).
 *
 * @param apci - the application control field
 * @param senderSeqNo - the sequence number of the sender
 */
void sb_process_direct_tel(unsigned short apci, unsigned short senderSeqNo)
{
    const unsigned int ackObjNo = SB_OBJ_NCD_ACK | senderSeqNo;
    const unsigned short senderAddr = (sbRecvTelegram[1] << 8) | sbRecvTelegram[2];
    static unsigned short dummy = 0;
    unsigned short count, address;

    if (sbConnectedAddr != senderAddr) // ensure that the sender is correct
        return;

    switch (apci & SB_APCI_GROUP_MASK)  // ADC / memory commands use the low bits for data
    {
    case SB_ADC_READ_PDU:
        address = sbRecvTelegram[7] & 0x3f;  // ADC channel
        count = sbRecvTelegram[8];           // number of samples
        sb_send_obj_value(ackObjNo);
        sb_send_obj_value(SB_ADC_RESPONSE | (count << 16) | address);
        return;

    case SB_MEMORY_READ_PDU:
        count = sbRecvTelegram[7] & 0x0f; // number of data byes
        address = (sbRecvTelegram[8] << 8) | sbRecvTelegram[9]; // address of the data block
        sb_send_obj_value(ackObjNo);
        sb_send_obj_value(SB_MEMORY_RESPONSE | (count << 16) | address);
        return;

    case SB_MEMORY_WRITE_PDU:
        count = sbRecvTelegram[7] & 0x0f; // number of data byes
        address = (sbRecvTelegram[8] << 8) | sbRecvTelegram[9]; // address of the data block
        sb_send_obj_value(ackObjNo);
        sb_update_memory(count, address, sbRecvTelegram + 10);
        return;
    }

    switch (apci)
    {
    case SB_DEVICEDESCRIPTOR_READ_PDU:
        sb_send_obj_value(ackObjNo);
        sb_send_obj_value(SB_READ_MASK_VERSION_RESPONSE | 0x12); // mask version: 0x12,0x13: BCU1, 0x20: BCU2
        return;

    case SB_RESTART_PDU:
        NVIC_SystemReset();  // Software Reset
        return;

    case SB_AUTHORIZE_REQUEST_PDU:
        sb_send_obj_value(ackObjNo);
        sb_send_obj_value(SB_AUTHORIZE_RESPONSE | 0);
    }

    ++dummy;  // to allow a breakpoint here
}

/**
 * Process a unicast connection control telegram with our physical address as
 * destination address. The telegram is stored in sbRecvTelegram[].
 *
 * When this function is called, the sender address is != 0 (not a broadcast).
 *
 * @param tpci - the transport control field
 */
void sb_process_connctrl_tel(unsigned char tpci)
{
    unsigned short senderAddr = (sbRecvTelegram[1] << 8) | sbRecvTelegram[2];

    if (tpci & 0x40)  // An acknowledgement
    {
        tpci &= 0xc3;
        if (tpci == SB_ACK_PDU) // A positive acknowledgement
        {
            if (sbIncConnectedSeqNo && sbConnectedAddr == senderAddr)
            {
                sbConnectedSeqNo += 4;
                sbConnectedSeqNo &= 0x3c;
            }
        }
        else if (tpci == SB_NACK_PDU)  // A negative acknowledgement
        {
            if (sbConnectedAddr == senderAddr)
                sb_send_obj_value(SB_T_DISCONNECT);
        }

        sbIncConnectedSeqNo = 0;
    }
    else  // A connect/disconnect command
    {
        if (tpci == SB_CONNECT_PDU)  // Open a direct data connection
        {
            if (sbConnectedAddr == 0)
            {
                sbConnectedAddr = senderAddr;
                sbConnectedSeqNo = 0;
                sbIncConnectedSeqNo = 0;
            }
        }
        else if (tpci == SB_DISCONNECT_PDU)  // Close the direct data connection
        {
            if (sbConnectedAddr == senderAddr)
            {
                sbConnectedAddr = 0;
                sbIncConnectedSeqNo = 0;
            }
        }
    }
}

/**
 * Process a multicast group telegram.
 * The telegram is stored in sbRecvTelegram[].
 *
 * @param destAddr - the destination group address.
 */
void sb_process_group_tel(unsigned short destAddr, unsigned char apci)
{
    unsigned short objno;
    unsigned short objflags;
    unsigned short gapos;
    unsigned short atp;
    unsigned short assmax;
    unsigned short asspos;

    // convert the group address into the index into the group address table
    gapos = sb_grp_address2index(destAddr);

    if (gapos != SB_INVALID_GRP_ADDRESS_IDX)
    {
        atp    = eeprom[SB_EEP_ASSOCTABPTR];                  // Base address of the assoc table
        assmax = atp + eeprom[atp] * SB_ASSOC_ENTRY_SIZE;     // first entry is the number of assoc's in the table

        // loop over all entry -> one group address could be assigned to multiple com objects
        for (asspos = atp + 1; asspos < assmax; asspos += 2)
        {
            // check of grp-address index in assoc table matches the dest grp address index
            if (gapos == eeprom[asspos]) // we found a assoc for our destAddr
            {
                objno    = eeprom[asspos + 1];         // get the com object number from the assoc table
                objflags = sb_read_objflags(objno); // get the flags for this com-object

                // check if it is a group write request
                if ((apci & 0xC0) == SB_WRITE_GROUP_REQUEST)
                {
                    // check if
                    // - communication is enabled (bit 2)
                    // - write is enabled         (bit 4)
                    if ((objflags & SB_COMOBJ_CONF_READ_COMM) == SB_COMOBJ_CONF_READ_COMM)
                        sb_write_value_req(objno);
                }
                if (apci == SB_READ_GROUP_REQUEST)
                {
                    // check if
                    // - communication is enabled (bit 2)
                    // - read  is enabled         (bit 3)
                    if ((objflags & SB_COMOBJ_CONF_WRITE_COMM) == SB_COMOBJ_CONF_WRITE_COMM)
                        sb_read_value_req(objno);  // read object value and send read_value_response 00000000 00000000
                    break;
                }
            }
        }
    }
}

/**
 * Process the received telegram in sbRecvTelegram[]. Call this function when sbRecvTelegramLen > 0.
 * After this function, sbRecvTelegramLen shall/will be zero.
 */
void sb_process_tel()
{
    unsigned short destAddr = (sbRecvTelegram[3] << 8) | sbRecvTelegram[4];
    unsigned char tpci = sbRecvTelegram[6] & 0xc3; // Transport control field (see KNX 3/3/4 p.6 TPDU)
    unsigned short apci = sbRecvTelegram[7] | ((sbRecvTelegram[6] & 3) << 8);

    if (destAddr == 0) // a broadcast
    {
        if (sb_prog_mode_active()) // we are in programming mode
        {
            if (tpci == SB_BROADCAST_PDU_SET_PA_REQ && apci == SB_INDIVIDUAL_ADDRESS_WRITE_PDU)
            {
                sb_set_pa((sbRecvTelegram[8] << 8) | sbRecvTelegram[9]);
            }
            else if (tpci == SB_BROADCAST_PDU_READ_PA && apci == SB_INDIVIDUAL_ADDRESS_READ_PDU)
                sb_send_obj_value(SB_INDIVIDUAL_ADDRESS_RESPONSE);
        }
    }
    else if ((sbRecvTelegram[5] & 0x80) == 0) // a physical destination address
    {
        if (destAddr == sbOwnPhysicalAddr) // it's our physical address
        {
            if (tpci & 0x80)  // A connection control command
            {
                sb_process_connctrl_tel(sbRecvTelegram[6]);
            }
            else
            {
                sb_process_direct_tel(apci, sbRecvTelegram[6] & 0x3c);
            }
        }
    }
    else if (tpci == SB_GROUP_PDU) // a group destination address and multicast
    {
        sb_process_group_tel(destAddr, apci);
    }

    // At the end: mark the received-telegram buffer as empty
    sbRecvTelegramLen = 0;
}

/**
 * Set our physical address
 *
 * @param addr - the physical address
 */
void sb_set_pa(unsigned short addr)
{
    eeprom[SB_EEP_ADDRTAB + 1] = addr >> 8;
    eeprom[SB_EEP_ADDRTAB + 1] = addr & 0xFF;
    sbOwnPhysicalAddr       = addr;

    sb_eeprom_update();
}

/**
 * Initialize the protocol.
 */
void sb_init_proto()
{
    sbConnectedAddr = 0;
}

/**
 * Add a com object to the ring-buffer for sending.
 *
 * @param objno - the number of the com object to send.
 * @return 1 if the com object was stored in the ring-buffer, 0 if
 *         the ring buffer is currently full.
 */
short sb_send_obj_value(unsigned int objno)
{
    if ((objno & SB_SEND_UNICAST_CMD_MASK) == 0 &&
        (sb_read_objflags(objno & 0xff) & SB_COMOBJ_CONF_TRANS_COMM) != SB_COMOBJ_CONF_TRANS_COMM)
    {
        // Do nothing if it is a (standard) com object but transmit or communication is disabled
    }
    else if (sbSendRingRead != ((sbSendRingWrite + 1) & (SB_SEND_RING_SIZE - 1)))
    {
        sbSendRing[sbSendRingWrite] = objno;
        ++sbSendRingWrite;
        sbSendRingWrite &= (SB_SEND_RING_SIZE - 1);
    }
    else
    {
        // The ring-buffer is full
        return 0;
    }

    return 1;
}

/**
 * Send the next telegram of the sending ring buffer.
 */
void sb_send_next_tel()
{
    if (sb_send_ring_empty())
        return;

    short i, length;
    unsigned short addr, destAddr, objType;
    unsigned long objValue = 1;  // FIXME dummy value for group telegram

    unsigned int objno = sbSendRing[sbSendRingRead];
    ++sbSendRingRead;
    sbSendRingRead &= (SB_SEND_RING_SIZE - 1);

    unsigned int cmd = objno & SB_SEND_CMD_MASK;

    if ((cmd & SB_SEND_UNICAST_CMD_MASK) == 0)  // send a com-object with a group telegram
    {
        objno &= 0xff;

        destAddr = 0x1102;  // dummy dest address: 2/1/2

        sbSendTelegram[0] = 0xbc; // Control byte
        // 1+2 contain the sender address, which is set by sb_send_tel()
        sbSendTelegram[3] = destAddr >> 8;
        sbSendTelegram[4] = destAddr;
        sbSendTelegram[6] = 0;

        if (cmd == SB_READ_VALUE_RESPONSE) sbSendTelegram[7] = 0x40; // ReadValue.response telegram
        else sbSendTelegram[7] = 0x80; // WriteValue.request telegram

        objType = 1; // FIXME dummy object type

        if (objType > 6) length = objType - 5;
        else length = 1;
        sbSendTelegram[5] = 0xe0 | length;

        if (length <= 1)
        {
            sbSendTelegram[7] |= objValue & 0x3f;
        }
        else
        {
            for (; length >= 0; --length)
            {
                sbSendTelegram[6 + length] = objValue;
                objValue >>= 8;
            }
        }
    }
    else  // handle a command for unicast/broadcast sending
    {
        sbSendTelegram[0] = 0xb0; // Control byte
        // 1+2 contain the sender address, which is set by sb_send_tel()
        sbSendTelegram[3] = sbConnectedAddr >> 8;
        sbSendTelegram[4] = sbConnectedAddr;

        switch (cmd)
        {
        case SB_OBJ_NCD_ACK:
            sbSendTelegram[5] = 0x60;
            sbSendTelegram[6] = 0xc2 | (objno & 0x3c);
            break;

        case SB_T_DISCONNECT:
            sbSendTelegram[5] = 0x60;
            sbSendTelegram[6] = 0x81;
            sbConnectedAddr = 0;
            break;

        case SB_READ_MASK_VERSION_RESPONSE:
            sbSendTelegram[5] = 0x63;
            sbSendTelegram[6] = 0x43 | sbConnectedSeqNo;
            sbSendTelegram[7] = 0x40;
            sbSendTelegram[8] = objno >> 8; // mask version (high byte)
            sbSendTelegram[9] = objno;      // mask version (low byte)
            sbIncConnectedSeqNo = 1;
            break;

        case SB_INDIVIDUAL_ADDRESS_RESPONSE:
            sbSendTelegram[3] = 0x00;  // Zero target address, it's a broadcast
            sbSendTelegram[4] = 0x00;
            sbSendTelegram[5] = 0xe1;
            sbSendTelegram[6] = 0x01;  // SB_INDIVIDUAL_ADDRESS_RESPONSE_PDU
            sbSendTelegram[7] = 0x40;
            break;

        case SB_MEMORY_RESPONSE:
            addr = objno & 0xffff;
            length = (objno >> 16) & 0x3f;
            for (i = 0; i < length; ++i)
            {
                if (addr & 0xff00)
                    sbSendTelegram[10 + i] = eeprom[(addr & 0xff) + i];
                else sbSendTelegram[10 + i] = userram[addr + i];
            }
            sbSendTelegram[5] = 0x63 + length;
            sbSendTelegram[6] = 0x42 | sbConnectedSeqNo;
            sbSendTelegram[7] = 0x40 | length;
            sbSendTelegram[8] = addr >> 8;
            sbSendTelegram[9] = addr;
            break;

        case SB_ADC_RESPONSE:
            sbSendTelegram[5] = 0x64;
            sbSendTelegram[6] = 0x41 | sbConnectedSeqNo;
            sbSendTelegram[7] = 0xc0 | (objno & 0x3f); // channel
            sbSendTelegram[8] = (objno >> 16) & 0xff;  // read count
            sbSendTelegram[9] = 0x05;                  // FIXME dummy - value high byte
            sbSendTelegram[10] = 0xb0;                 // FIXME dummy - value low byte
            break;

        case SB_AUTHORIZE_RESPONSE:
            sbSendTelegram[5] = 0x62;
            sbSendTelegram[6] = 0x43 | sbConnectedSeqNo;
            sbSendTelegram[7] = 0xd2;
            sbSendTelegram[7] = 0x00;
            sbIncConnectedSeqNo = 1;
            break;

        default:
            // ignore unknown command
            return;
        }
    }

    sbIncConnectedSeqNo = sbSendTelegram[6] & 0x40;
    sb_send_tel(7 + (sbSendTelegram[5] & 0x0f));
}

static void sb_update_memory(signed char count, unsigned short address, unsigned char * data)
{
    unsigned char * mem        = userram;
    unsigned char   update_ram = 1;
    if (address &  0xFF00)
    {
        mem        = eeprom;
        address   &= 0xFF;
        update_ram = 0;
    }

    while (count--)
    {
        // count the data for address 0x60 to the status variable as well
        if (update_ram && (address == 0x60))
            sbStatus = * data;
        mem [address++] = * data++;
    }
    if (!update_ram)
    {
        //sb_eeprom_update();
    }
}

static unsigned short sb_grp_address2index(unsigned short address)
{
    unsigned short ga_position = SB_INVALID_GRP_ADDRESS_IDX;
    unsigned char n;
    unsigned char gah = address >> 8;;
    unsigned char gal = address &  0xFF;

    if (eeprom[SB_EEP_ADDRTAB] < 0xFF) // && !transparency)
    {
        if (eeprom[SB_EEP_ADDRTAB])
        {
            for (n = eeprom[SB_EEP_ADDRTAB] - 1; n; n--)
            {
                if (  (gah == eeprom[SB_EEP_ADDRTAB + n * 2 + 1])
                   && (gal == eeprom[SB_EEP_ADDRTAB + n * 2 + 2])
                   )
                    ga_position = n;
            }
        }
    }
    return (ga_position);
}

/**
 * Read the object flags.
 *
 * @param objno - the number of the com object
 * @return The object flags of the com object.
 */
unsigned char sb_read_objflags(unsigned short objno)
{
    return (eeprom[eeprom[SB_EEP_COMMSTABPTR] + 3 + 3 * objno]);
}
