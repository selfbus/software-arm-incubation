// sb_proto.c

#include "sb_proto.h"
#include "sb_bus.h"
#include "sb_const.h"
#include "sb_eep_emu.h"

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
 */
static void sb_process_direct_tel(unsigned char tpci, unsigned char apci)
{
    unsigned short senderAddr = (sbRecvTelegram[1] << 8) | sbRecvTelegram[2];
    unsigned short senderSeqNo = sbRecvTelegram[6] & 0x3c;

    // See fb_lpc922.c line 547..599 for example implementation
    switch (tpci)
    {
    // Memory operations in connected data mode
    case SB_DATA_PDU_MEMORY_OPERATIONS:
        if (sbConnectedAddr == senderAddr) // ensure that the sender is correct
        {
            signed char count = sbRecvTelegram[7] & 0x0F; // number of data byes
            unsigned short address = (sbRecvTelegram[8] << 8) | sbRecvTelegram[9]; // address of the data block

            apci &= 0xF0;                               // on memory operations only the high nibble is used
            if (apci == SB_WRITE_MEMORY_REQUEST)        // 01pppp10 1000xxxx
            {
                sb_send_obj_value(SB_OBJ_NCD_ACK | senderSeqNo);
                sb_update_memory(count, address, sbRecvTelegram + 10);
            }
            if (apci == SB_READ_MEMORY_REQUEST)         // 01pppp10 0000xxxx
            {
                sb_send_obj_value(SB_OBJ_NCD_ACK | senderSeqNo);
                sb_send_obj_value(SB_READ_MEMORY_REQUEST | (count << 16) | address);
            }
        }
        break;

    // Misc operations
    case SB_DATA_PDU_MISC_OPERATIONS:
        if (apci == SB_RESTART_REQUEST)                   // 01pppp11 10000000
        {
            NVIC_SystemReset(); // Software Reset
        }
        if (apci == SB_READ_MASK_VERSION_REQUEST)        // 01pppp11 00000000
        {
            sb_send_obj_value(SB_OBJ_NCD_ACK | senderSeqNo);
            sb_send_obj_value(SB_READ_MASK_VERSION_REQUEST);
        }
        break;

    // Open a direct data connection
    case SB_CONNECT_PDU:
        if (sbConnectedAddr == 0)
        {
            sbConnectedAddr = senderAddr;
            sbConnectedSeqNo = 0;
            senderSeqNo = 0;
        }
        break;

    // Close the direct data connection
    case SB_DISCONNECT_PDU:
        if (sbConnectedAddr == senderAddr) // only close connection if the sender is correct
        {
            sbConnectedAddr = 0;
        }
        break;

    case SB_NACK_PDU:
        if (sbConnectedAddr == senderAddr) // only close connection if the sender is correct
        {
            sb_send_obj_value(SB_T_DISCONNECT); // Send disconnect
        }
        break;
    }
}

/**
 * Process a multicast group telegram.
 * The telegram is stored in sbRecvTelegram[].
 *
 * @param destAddr - the destination group address.
 */
static void sb_process_group_tel(unsigned short destAddr, unsigned char apci)
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
        atp    = eep[SB_EEP_ASSOCTABPTR];                  // Base address of the assoc table
        assmax = atp + eep[atp] * SB_ASSOC_ENTRY_SIZE;     // first entry is the number of assoc's in the table

        // loop over all entry -> one group address could be assigned to multiple com objects
        for (asspos = atp + 1; asspos < assmax; asspos += 2)
        {
            // check of grp-address index in assoc table matches the dest grp address index
            if (gapos == eep[asspos]) // we found a assoc for our destAddr
            {
                objno    = eep[asspos + 1];         // get the com object number from the assoc table
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
    unsigned char tpci = sbRecvTelegram[6] & 0xC3; // See KNX 3/3/4 p.6 TPDU
    unsigned char apci = sbRecvTelegram[7];

    if (destAddr == 0) // a broadcast
    {
        if (sb_prog_mode_active()) // we are in programming mode
        {
            if (tpci == SB_BROADCAST_PDU_SET_PA_REQ && apci == SB_SET_PHYSADDR_REQUEST)
            {
                sb_set_pa((sbRecvTelegram[8] << 8) | sbRecvTelegram[9]);
            }
            else if (tpci == SB_BROADCAST_PDU_READ_PA && apci == SB_READ_PHYSADDR_REQUEST)
                sb_send_obj_value(SB_READ_PHYSADDR_RESPONSE);
        }
    }
    else if ((sbRecvTelegram[5] & 0x80) == 0) // a physical destination address
    {
        if (destAddr == sbOwnPhysicalAddr) // it's our physical address
        {
            sb_process_direct_tel(tpci, apci);
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
    eep[SB_EEP_ADDRTAB + 1] = addr >> 8;
    eep[SB_EEP_ADDRTAB + 1] = addr & 0xFF;

    sb_eep_update();
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

    if ((objno & SB_SEND_UNICAST_CMD_MASK) == 0)  // send a com-object with a group telegram
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
    else  // handle a command for unicast sending
    {
        sbSendTelegram[0] = 0xb0; // Control byte
        // 1+2 contain the sender address, which is set by sb_send_tel()
        sbSendTelegram[3] = sbConnectedAddr >> 8;
        sbSendTelegram[4] = sbConnectedAddr;

        switch (objno)
        {
        case SB_OBJ_NCD_ACK:
            sbSendTelegram[5] = 0x60;
            sbSendTelegram[6] = 0xc2 | (objno & 0xff);
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
            sbSendTelegram[8] = 0x00;
            sbSendTelegram[9] = 0x12; // mask version 1.2: BCU1
            ++sbConnectedSeqNo; // TODO increment only on successful sending
            break;

        case SB_READ_PHYSADDR_RESPONSE:
            sbSendTelegram[3] = 0x00; // zero target address, it's a broadcast
            sbSendTelegram[4] = 0x00;
            sbSendTelegram[5] = 0xe1;
            sbSendTelegram[6] = 0x01;
            sbSendTelegram[7] = 0x40;
            break;

        case SB_READ_MEMORY_RESPONSE:
            addr = objno & 0xffff;
            length = (objno >> 16) & 0x0f;
            for (i = 0; i < length; ++i, ++addr)
            {
                if (addr & 0xff00)
                    sbSendTelegram[10 + i] = eep[addr & 0xff];
                else sbSendTelegram[10 + i] = userram[addr];
            }
            sbSendTelegram[5] = 0x63 + length;
            sbSendTelegram[6] = 0x42 | sbConnectedSeqNo;
            sbSendTelegram[7] = 0x40 | length;
            sbSendTelegram[8] = addr >> 8;
            sbSendTelegram[9] = addr;
            ++sbConnectedSeqNo; // TODO increment only on successful sending
            break;

        case SB_READ_ADC_RESPONSE:
            // TODO implement SB_READ_ADC_RESPONSE
            break;

        default:
            // ignore unknown command
            return;
        }
    }

    sb_send_tel(7 + (sbSendTelegram[5] & 0x0f));
}

static void sb_update_memory(signed char count, unsigned short address, unsigned char * data)
{
    while (count--)
    {
        // count the data for address 0x60 to the status variable as well
        if (address == 0x60)
            sbStatus = * data;
        eep [address++] = * data++;
    }
    //sb_eep_update();
}

static unsigned short sb_grp_address2index(unsigned short address)
{
    unsigned short ga_position = SB_INVALID_GRP_ADDRESS_IDX;
    unsigned char n;
    unsigned char gah = address >> 8;;
    unsigned char gal = address &  0xFF;

    if (eep[SB_EEP_ADDRTAB] < 0xFF) // && !transparency)
    {
        if (eep[SB_EEP_ADDRTAB])
        {
            for (n = eep[SB_EEP_ADDRTAB] - 1; n; n--)
            {
                if (  (gah == eep[SB_EEP_ADDRTAB + n * 2 + 1])
                   && (gal == eep[SB_EEP_ADDRTAB + n * 2 + 2])
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
    return (eep[eep[SB_EEP_COMMSTABPTR] + 3 + 3 * objno]);
}
