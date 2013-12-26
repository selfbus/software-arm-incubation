/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
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

    // Send the first bit of the current byte
    SB_SEND_BIT_0,

    // Send the bits of the current byte
    SB_SEND_BYTE,

    // Wait between two sendings
    SB_SEND_WAIT,

    // Finish sending
    SB_SEND_END
};

// The state of the lib's receiver/sender
extern enum SbState sbState;

// The size of the telegram buffer in bytes
#define SB_TELEGRAM_SIZE 24

/**
 * The telegram buffer.
 */
extern unsigned char sbRecvTelegram[SB_TELEGRAM_SIZE];

/**
 * The length of the received telegram in bytes, including the checksum.
 * Zero if no telegram was received.
 */
extern unsigned short sbRecvTelegramLen;

/**
 * The telegram buffer for sending a telegram.
 */
extern unsigned char sbSendTelegram[SB_TELEGRAM_SIZE];

/**
 * The telegram that is currently being sent.
 */
extern unsigned char *sbSendCurTelegram;

/**
 * The telegram to be sent after sbSendTelegram is done.
 */
extern unsigned char *sbSendNextTelegram;

/**
 * Our own physical address on the bus.
 */
extern unsigned short sbOwnPhysicalAddr;

/**
 * Set the wakeup timer when the bus is idle.
 *
 * @param timeout - the timeout in usec between timer wakeups.
 *
 * @brief When the bus is idle, a bus-timer interrupt is generated regularily.
 * Use this function to set the time between wakeups. The timer is a 16bit
 * timer with an active prescaler. For 48MHz system clock the maximum available
 * time is 5400 usec. If the given time is too high, the maximum available timer
 * value is used instead.
 */
void sb_set_wakeup_time(unsigned short timeout);

/**
 * Initialize the bus access.
 */
void sb_init_bus();

/**
 * Prepare the telegram for sending. Set the sender address to our own
 * address, and calculate the checksum of the telegram.
 * Stores the checksum at telegram[length].
 *
 * @param telegram - the telegram to process
 * @param length - the length of the telegram
 */
void sb_prepare_tel(unsigned char* telegram, unsigned short length);

/**
 * Send a telegram. The checksum byte will be added at the end of telegram[].
 * Ensure that there is at least one byte space at the end of telegram[].
 *
 * @param telegram - the telegram to be sent.
 * @param length - the length of the telegram in sbSendTelegram[], without the checksum
 */
void sb_send_tel(unsigned char* telegram, unsigned short length);

/**
 * Test if we are in programming mode (the button on the controller is pressed and
 * the red programming LED is on).
 *
 * @return 1 if in programming mode, 0 if not.
 */
#define sb_prog_mode_active() (sbUserRam->status & 1)

/**
 * Get the length of a telegram, including the protocol header but excluding
 * the checksum byte.
 *
 * @param telegram - the telegram to query
 *
 * @return The length of the telegram, excluding the checksum byte.
 */
#define sb_tel_length(telegram) (7 + (telegram[5] & 15))

#endif /*sb_bus_h*/
