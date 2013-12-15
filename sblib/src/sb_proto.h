/**
 * Low level EIB protocol handling
 */
#ifndef sb_proto_h
#define sb_proto_h

/**
 * Process the received telegram in sbRecvTelegram[]. Call this function when sbRecvTelegramLen > 0.
 * After this function, sbRecvTelegramLen shall/will be zero.
 */
extern void sb_process_tel();

/**
 * Set our physical address.
 *
 * @param addr - the physical address
 */
extern void sb_set_pa(unsigned short addr);

/**
 * Initialize the protocol.
 */
extern void sb_init_proto();

/**
 * Test if there is a direct data connection active.
 *
 * @return 1 if a direct data connection is active, 0 if not.
 */
#define sb_connected() (sbConnectedAddr != 0)


/**
 * Physical address of the remote device when a direct data connection is active.
 * Zero if no connection is active.
 */
extern unsigned short sbConnectedAddr;



/**
 * @return The number of send requests in the sending ring buffer sbSendRing.
 */
#define sb_send_ring_count() ((sbSendRingWrite - sbSendRingRead) & 7)


/**
 * The size of the ring buffer for send requests.
 */
#define SB_SEND_RING_SIZE 8

/**
 * Ring buffer for send requests.
 */
extern unsigned short sbSendRing[SB_SEND_RING_SIZE];

/**
 * Index in sbSendRing[] where the next write will occur.
 */
extern unsigned short sbSendRingWrite;

/**
 * Index in sbSendRing[] where the next read will occur.
 */
extern unsigned short sbSendRingRead;

/**
 * @return The number of send requests in the sending ring buffer sbSendRing.
 */
#define sb_send_ring_count() ((sbSendRingWrite - sbSendRingRead) & 7)


#endif /*sb_proto_h*/
