/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *                     Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sb_proto_h
#define sb_proto_h

/**
 * Process the received telegram in sbRecvTelegram[]. Call this function when sbRecvTelegramLen > 0.
 * After this function, sbRecvTelegramLen shall/will be zero.
 */
void sb_process_tel();

#ifdef OLD_CODE
/**
 * Send the next telegram of the sending ring buffer.
 */
void sb_send_next_tel();
#endif

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
 * Indicates that EEPROM memory was written and needs to be flushed.
 */
extern unsigned short sbEepromDirty;

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

unsigned long sb_read_obj_value(unsigned short objno);

#endif /*sb_proto_h*/
