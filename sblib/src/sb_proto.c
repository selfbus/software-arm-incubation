// sb_proto.c

#include "sb_proto.h"
#include "sb_bus.h"
#include "sb_const.h"
#include "sb_eep_emu.h"


// Ring buffer for send requests.
unsigned short sbSendRing[SB_SEND_RING_SIZE];

// Index in sbSendRing[] where the next write will occur.
extern unsigned short sbSendRingWrite;

// Index in sbSendRing[] where the next read will occur.
extern unsigned short sbSendRingRead;




/**
 * Process a telegram with our physical address as destination address.
 * The telegram is stored in sbRecvTelegram[].
 */
static void sb_process_phys_tel()
{
    unsigned char tpdu = sbRecvTelegram[6] & 0xc3;

    switch (tpdu)
    {
        // TODO see fb_lpc922.c line 547..599 for example implementation
    }
}

/**
 * Process the received group telegram in sbRecvTelegram[]. This function is called by
 * sb_process_tel() and must be implemented.
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
    unsigned short isGroupDestAddr = (sbRecvTelegram[5] & 0x80) == 0;
    unsigned char tpdu = sbRecvTelegram[6] & 0xc3;
    unsigned char apdu = sbRecvTelegram[7];

    if (!destAddr) // A broadcast?
    {
        if (sb_prog_mode_active()) // A broadcast and we are in programming mode
        {
            if (tpdu == SB_BROADCAST_PDU_SET_PA_REQ && apdu == SB_SET_PHYSADDR_REQUEST)
            {
                sb_set_pa((sbRecvTelegram[8] << 8) | sbRecvTelegram[9]);
            }

            // TODO
//            else if (tpdu == SB_BROADCAST_PDU_READ_PA && apdu == SB_READ_PHYSADDR_REQUEST)
//                sb_send_obj_value(READ_PHYSADDR_RESPONSE);

        }
    }
    else if (!isGroupDestAddr) // A physical destination address?
    {
        if (destAddr == sbOwnPhysicalAddr) // is it our address?
        {
            sb_process_phys_tel();
        }
    }
    else if (tpdu == SB_GROUP_PDU) // A group destination address with multicast?
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
