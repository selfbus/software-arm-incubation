/**
 * Low level EIB protocol handling
 */
#ifndef sb_proto_h
#define sb_proto_h

/**
 * Process the received telegram in sbRecvTelegram[]. Call this function when sbRecvTelegramLen > 0.
 * After this function, sbRecvTelegramLen shall/will be zero.
 */
void sb_process_tel();

/**
 * Send the next telegram of the sending ring buffer.
 */
void sb_send_next_tel();

/**
 * Set our physical address.
 *
 * @param addr - the physical address
 */
void sb_set_pa(unsigned short addr);

/**
 * Initialize the protocol.
 */
void sb_init_proto();

/**
 * Add a com object to the sending ring buffer.
 *
 * @param objno - the number of the com object to send. See sb_const.h for constants for the
 *                com object. E.g. SB_READ_VALUE_RESPONSE to mark the object as a reply.
 * @return 1 if the com object was stored in the ring-buffer,
 *         0 if the ring buffer is currently full.
 */
short sb_send_obj_value(unsigned int objno);

/**
 * Read the object flags.
 *
 * @param objno - the number of the com object
 * @return The object flags of the com object.
 */
unsigned char sb_read_objflags(unsigned short objno);

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
#define sb_send_ring_count() ((sbSendRingWrite - sbSendRingRead) & (SB_SEND_RING_SIZE - 1))

/**
 * Test if the sending ring buffer is empty.
 *
 * @return 1 if empty, 0 if not.
 */
#define sb_send_ring_empty() (sbSendRingWrite == sbSendRingRead)


/**
 * The size of the ring buffer for send requests.
 * Must be power of 2.
 */
#define SB_SEND_RING_SIZE 8

/**
 * Ring buffer for send requests.
 */
extern unsigned int sbSendRing[SB_SEND_RING_SIZE];

/**
 * Index in sbSendRing[] where the next write will occur.
 */
extern unsigned short sbSendRingWrite;

/**
 * Index in sbSendRing[] where the next read will occur.
 */
extern unsigned short sbSendRingRead;

void sb_write_value_req(unsigned short objno);
void sb_read_value_req(unsigned short objno);

#endif /*sb_proto_h*/
