/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sb_proto_p_h
#define sb_proto_p_h

//
// This is the library internal header file for sb_proto.c
//


/**
 * Send a connection control telegram.
 *
 * @param cmd - the transport command, see SB_T_xx defines
 * @param senderSeqNo - the sequence number of the sender
 */
void sb_send_con_ctrl_tel(unsigned char cmd, unsigned char senderSeqNo);

/**
 * Process a unicast connection control telegram with our physical address as
 * destination address. The telegram is stored in sbRecvTelegram[].
 *
 * When this function is called, the sender address is != 0 (not a broadcast).
 *
 * @param tpci - the transport control field
 */
void sb_process_con_ctrl_tel(unsigned char tpci);



#endif /*sb_proto_p_h*/
