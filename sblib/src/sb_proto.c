// sb_proto.c

#include "sb_proto.h"
#include "sb_bus.h"
#include "sb_const.h"
#include "sb_eep_emu.h"


// Ring buffer for send requests.
unsigned short sbSendRing[SB_SEND_RING_SIZE];

// Index in sbSendRing[] where the next write will occur.
unsigned short sbSendRingWrite;

// Index in sbSendRing[] where the next read will occur.
unsigned short sbSendRingRead;

// Physical address of the remote device when a direct data connection is active.
// Zero if no connection is active.
unsigned short sbConnectedAddr;

// Sequence number for sending telegrams
unsigned char sbConnectedSeqNo;


/**
 * Process a unicast telegram with our physical address as destination address.
 * The telegram is stored in sbRecvTelegram[].
 *
 * When this function is called, the sender address is != 0 (not a broadcast).
 */
static void sb_process_direct_tel()
{
    unsigned char tpci = sbRecvTelegram[6] & 0xc3;
    unsigned short senderAddr = (sbRecvTelegram[1] << 8) | sbRecvTelegram[2];
    unsigned char senderSeqNo = sbRecvTelegram[6] & 0x3c;

    // See fb_lpc922.c line 547..599 for example implementation
    switch (tpci)
    {
    // Memory operations in connected data mode
    case SB_DATA_PDU_MEMORY_OPERATIONS:
        if (sbConnectedAddr == senderAddr) // ensure that the sender is correct
        {

        }
        break;

    // Misc operations
    case SB_DATA_PDU_MISC_OPERATIONS:
        break;

    // Open a direct data connection
    case SB_CONNECT_PDU:
        if (sbConnectedAddr == 0)
        {
            sbConnectedAddr = senderAddr;
            sbConnectedSeqNo = 0;
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
        // Send T_DISCONNECT
        break;
    }
}

/**
 * Process a multicast group telegram.
 * The telegram is stored in sbRecvTelegram[].
 *
 * @param destAddr - the destination group address.
 */
static void sb_process_group_tel(unsigned short destAddr)
{
    // TODO process group telegrams
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
            // TODO
            // else if (tpdu == SB_BROADCAST_PDU_READ_PA && apdu == SB_READ_PHYSADDR_REQUEST)
            //     sb_send_obj_value(READ_PHYSADDR_RESPONSE);
        }
    }
    else if ((sbRecvTelegram[5] & 0x80) == 0) // a physical destination address
    {
        if (destAddr == sbOwnPhysicalAddr) // it's our physical address
        {
            sb_process_direct_tel();
        }
    }
    else if (tpci == SB_GROUP_PDU) // a group destination address and multicast
    {
        sb_process_group_tel(destAddr);
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
