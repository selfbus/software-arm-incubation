/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *                     Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "sb_proto.h"
#include "internal/sb_proto_p.h"

#include "sb_bus.h"
#include "sb_const.h"
#include "sb_memory.h"
#include "sb_eeprom.h"
#include "sb_comobj.h"
#include "sb_props.h"
#include <string.h>

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif


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

// Buffer for sending a connected control telegram
unsigned char sbSendConCtrlTelegram[8];

// Indicates that EEPROM memory was written and needs to be flushed
unsigned short sbEepromDirty;


/**
 * Send a connection control telegram.
 *
 * @param cmd - the transport command, see SB_T_xx defines
 * @param senderSeqNo - the sequence number of the sender, 0 if not required
 */
void sb_send_con_ctrl_tel(unsigned char cmd, unsigned char senderSeqNo)
{
    if (cmd & 0x40)  // Add the sequence number if the command shall contain it
        cmd |= senderSeqNo & 0x3c;

    sbSendConCtrlTelegram[0] = 0xb0 | (sbRecvTelegram[0] & 0x0c); // Control byte
    // 1+2 contain the sender address, which is set by sb_send_tel()
    sbSendConCtrlTelegram[3] = sbConnectedAddr >> 8;
    sbSendConCtrlTelegram[4] = sbConnectedAddr;
    sbSendConCtrlTelegram[5] = 0x60;
    sbSendConCtrlTelegram[6] = cmd;

    sb_send_tel(sbSendConCtrlTelegram, 7);
}

/**
 * Process a device-descriptor-read request.
 *
 * @param id - the device-descriptor type ID
 *
 * @return 1 on success, 0 on failure
 */
unsigned short sb_process_device_descriptor_read(unsigned char id)
{
    switch (id)
    {
    case 0:
        sbSendTelegram[5] = 0x63;
        sbSendTelegram[6] = 0x43;
        sbSendTelegram[7] = 0x40;
#if defined(SB_BCU1)
        sbSendTelegram[8] = 0x00;  // mask version (high byte)
        sbSendTelegram[9] = 0x12;  // mask version (low byte)
#elif defined(SB_BCU2)
        sbSendTelegram[8] = 0x00;  // mask version (high byte)
        sbSendTelegram[9] = 0x20;  // mask version (low byte)
#else
#   error no BCU version defined?
#endif
        break;

    default:
        return 0; // unknown device descriptor
    }

    return 1;
}

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
    const unsigned short senderAddr = (sbRecvTelegram[1] << 8) | sbRecvTelegram[2];
    unsigned short count, address, index, id;
    unsigned char sendAck = 0;
    unsigned char sendTel = 0;

    if (sbConnectedAddr != senderAddr) // ensure that the sender is correct
        return;

    sbSendTelegram[6] = 0;

    switch (apci & SB_APCI_GROUP_MASK)  // ADC / memory commands use the low bits for data
    {
    case SB_ADC_READ_PDU:
        index = sbRecvTelegram[7] & 0x3f;  // ADC channel
        count = sbRecvTelegram[8];           // number of samples
        sbSendTelegram[5] = 0x64;
        sbSendTelegram[6] = 0x41;
        sbSendTelegram[7] = 0xc0 | (index & 0x3f);   // channel
        sbSendTelegram[8] = count;                   // read count
        sbSendTelegram[9] = 0x05;                    // FIXME dummy - value high byte
        sbSendTelegram[10] = 0xb0;                   // FIXME dummy - value low byte
        sendAck = SB_T_ACK_PDU;
        sendTel = 1;
        break;

    case SB_MEMORY_READ_PDU:
        sendAck = SB_T_ACK_PDU;
        sendTel = 1;
        count = sbRecvTelegram[7] & 0x0f; // number of data byes
        address = (sbRecvTelegram[8] << 8) | sbRecvTelegram[9]; // address of the data block
        if (address >= SB_EEPROM_START && address < SB_EEPROM_END)
            memcpy(sbSendTelegram + 10, sbEepromData + address - SB_EEPROM_START, count);
        else memcpy(sbSendTelegram + 10, sbUserRamData + address - SB_USERRAM_START, count);
        sbSendTelegram[5] = 0x63 + count;
        sbSendTelegram[6] = 0x42;
        sbSendTelegram[7] = 0x40 | count;
        sbSendTelegram[8] = address >> 8;
        sbSendTelegram[9] = address;
        break;

    case SB_MEMORY_WRITE_PDU:
        count = sbRecvTelegram[7] & 0x0f; // number of data byes
        address = (sbRecvTelegram[8] << 8) | sbRecvTelegram[9]; // address of the data block
        sendAck = SB_T_ACK_PDU;
        if (address >= SB_EEPROM_START && address < SB_EEPROM_END)
        {
            memcpy(sbEepromData + address - SB_EEPROM_START, sbRecvTelegram + 10, count);
            sbEepromDirty = 1;
        }
        else if (address >= SB_USERRAM_START && address < SB_USERRAM_END)
        {
            memcpy(sbUserRamData + address - SB_USERRAM_START, sbRecvTelegram + 10, count);
        }
        break;

    case SB_DEVICEDESCRIPTOR_READ_PDU:
        if (sb_process_device_descriptor_read(apci & 0x3f))
        {
            sendAck = SB_T_ACK_PDU;
            sendTel = 1;
        }
        else sendAck = SB_T_NACK_PDU;
        break;

    default:
        switch (apci)
        {
        case SB_RESTART_PDU:
            if (sbEepromDirty)   // Flush the EEPROM before resetting
                sb_eeprom_update();
            NVIC_SystemReset();  // Software Reset
            break;

        case SB_AUTHORIZE_REQUEST_PDU:
            sbSendTelegram[5] = 0x62;
            sbSendTelegram[6] = 0x43;
            sbSendTelegram[7] = 0xd2;
            sbSendTelegram[8] = 0x00;
            sendAck = SB_T_ACK_PDU;
            sendTel = 1;
            break;

        case SB_PROPERTY_VALUE_READ_PDU:
            index = sbSendTelegram[8] = sbRecvTelegram[8];
            id = sbSendTelegram[9] = sbRecvTelegram[9];
            count = (sbSendTelegram[10] = sbRecvTelegram[10]) >> 4;
            address = (sbRecvTelegram[10] << 4) | (sbSendTelegram[11] = sbRecvTelegram[11]);
            sb_props_read_tel(index, id, count, address);
            break;
        }
        break;
    }

    if (sendAck)
        sb_send_con_ctrl_tel(sendAck, senderSeqNo);

    if (sendTel)
    {
        sbSendTelegram[0] = 0xb0 | (sbRecvTelegram[0] & 0x0c); // Control byte
        // 1+2 contain the sender address, which is set by sb_send_tel()
        sbSendTelegram[3] = sbConnectedAddr >> 8;
        sbSendTelegram[4] = sbConnectedAddr;

        if (sbSendTelegram[6] & 0x40) // Add sequence number if applicable
        {
            sbSendTelegram[6] |= sbConnectedSeqNo;
            sbIncConnectedSeqNo = 1;
        }
        else sbIncConnectedSeqNo = 0;

        sb_send_tel(sbSendTelegram, sb_tel_length(sbSendTelegram));
    }
}

/**
 * Process a unicast connection control telegram with our physical address as
 * destination address. The telegram is stored in sbRecvTelegram[].
 *
 * When this function is called, the sender address is != 0 (not a broadcast).
 *
 * @param tpci - the transport control field
 */
void sb_process_con_ctrl_tel(unsigned char tpci)
{
    unsigned short senderAddr = (sbRecvTelegram[1] << 8) | sbRecvTelegram[2];

    if (tpci & 0x40)  // An acknowledgement
    {
        tpci &= 0xc3;
        if (tpci == SB_T_ACK_PDU) // A positive acknowledgement
        {
            if (sbIncConnectedSeqNo && sbConnectedAddr == senderAddr)
            {
                sbConnectedSeqNo += 4;
                sbConnectedSeqNo &= 0x3c;
                sbIncConnectedSeqNo = 0;
            }
        }
        else if (tpci == SB_T_NACK_PDU)  // A negative acknowledgement
        {
            if (sbConnectedAddr == senderAddr)
                sb_send_con_ctrl_tel(SB_T_DISCONNECT_PDU, 0);
        }

        sbIncConnectedSeqNo = 0;
    }
    else  // A connect/disconnect command
    {
        if (tpci == SB_T_CONNECT_PDU)  // Open a direct data connection
        {
            if (sbConnectedAddr == 0)
            {
                sbConnectedAddr = senderAddr;
                sbConnectedSeqNo = 0;
                sbIncConnectedSeqNo = 0;
            }
        }
        else if (tpci == SB_T_DISCONNECT_PDU)  // Close the direct data connection
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
            if (apci == SB_INDIVIDUAL_ADDRESS_WRITE_PDU)
            {
                sb_set_pa((sbRecvTelegram[8] << 8) | sbRecvTelegram[9]);
            }
            else if (apci == SB_INDIVIDUAL_ADDRESS_READ_PDU)
            {
                sbSendTelegram[0] = 0xb0 | (sbRecvTelegram[0] & 0x0c); // Control byte
                // 1+2 contain the sender address, which is set by sb_send_tel()
                sbSendTelegram[3] = 0x00;  // Zero target address, it's a broadcast
                sbSendTelegram[4] = 0x00;
                sbSendTelegram[5] = 0xe1;
                sbSendTelegram[6] = 0x01;  // SB_INDIVIDUAL_ADDRESS_RESPONSE_PDU
                sbSendTelegram[7] = 0x40;
                sb_send_tel(sbSendTelegram, 8);
            }
        }
    }
    else if ((sbRecvTelegram[5] & 0x80) == 0) // a physical destination address
    {
        if (destAddr == sbOwnPhysicalAddr) // it's our physical address
        {
            if (tpci & 0x80)  // A connection control command
            {
                sb_process_con_ctrl_tel(sbRecvTelegram[6]);
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
    sbEeprom->addrTab[0] = addr >> 8;
    sbEeprom->addrTab[1] = addr;
    sbEepromDirty = 1;

    sbOwnPhysicalAddr = addr;
}

/**
 * Initialize the protocol.
 */
void sb_init_proto()
{
    sbConnectedAddr = 0;
    sbEepromDirty = 0;
}


#ifdef OLD_CODE
/**
 * Send the next telegram of the sending ring buffer.
 */
void sb_send_next_tel()
{
    if (sb_send_ring_empty())
        return;

    short length;
    unsigned short destAddr, objType;
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

        sb_send_tel(sbSendTelegram, sb_tel_length(sbSendTelegram));
    }
}
#endif // OLD_CODE
