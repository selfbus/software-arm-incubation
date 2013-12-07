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

#endif /*sb_proto_h*/
