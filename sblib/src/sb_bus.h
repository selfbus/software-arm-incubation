/**
 * Low level EIB bus access functions.
 */
#ifndef sb_bus_h
#define sb_bus_h


// The state of the lib's telegram sending/receiving
enum SbState
{
	// The lib is idle. No receiving or sending.
	SB_IDLE = 0,

	// The lib is receiving a byte.
	SB_RECV_BYTE,

	// The lib is waiting for the start bit of the next byte.
	SB_RECV_START,

	// Start sending the telegram in sbSendTelegram[].
	SB_SEND_INIT,

	// Start sending the next byte of a telegram
	SB_SEND_START,

	// Send the bits of the current byte
	SB_SEND_BYTE,

	// Finish sending
	SB_SEND_END,

	// Switch to idle
	SB_ENTER_IDLE
};

// The state of the lib's receiver/sender
extern enum SbState sbState;

// The size of the telegram buffer in bytes
#define SB_TELEGRAM_SIZE 23

/**
 * The telegram buffer.
 */
extern unsigned char sbRecvTelegram[SB_TELEGRAM_SIZE];

/**
 * The length of the received telegram in bytes, including the checksum.
 * Zero if no telegram was received.
 */
unsigned short sbRecvTelegramLen;

/**
 * The telegram buffer for sending a telegram.
 */
extern unsigned char sbSendTelegram[SB_TELEGRAM_SIZE];

/**
 * The length of the telegram to be sent in bytes, including the checksum.
 * Zero if no telegram is being sent.
 */
unsigned short sbSendTelegramLen;


/**
 * Initialize the bus access.
 */
extern void sb_init_bus();

/**
 * Process the received telegram in sbTelegram[]. Call this function when sbTelegramLen > 0.
 * Afterwards, sbTelegramLen is zero again.
 */
extern void sb_process_tel();

/**
 * Send the telegram that is stored in sbSendTelegram[].
 * The function calculates the checksum and sets the sender address before sending.
 *
 * @param length - the length of the telegram in sbSendTelegram[], without the checksum
 */
extern void sb_send_tel(unsigned short length);


#endif /*sb_bus_h*/
