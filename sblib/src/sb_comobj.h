/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sb_comobj_h
#define sb_comobj_h

/**
 * Schedule a com-object for sending. This will send a group telegram
 * with the com-object's value.
 *
 * A group telegram is only sent if a group address exists for this
 * com-object and communication and transmit flags are set for the com-object.
 *
 * @param objno - the number of the com object to send.
 */
void sb_send_obj_value(unsigned short objno);

/**
 * Get the pointer to the value of a com-object.
 *
 * @param objno - the com-object to query
 * @return The pointer to the com-object's value, or 0 if objno is invalid
 */
void* sb_get_value_ptr(unsigned short objno);

/**
 * Get the communication (RAM) flags of a com-object.
 * See SB_COMFLAG_ defines below.
 *
 * @param objno - the com-object to query
 * @return The communication flags.
 */
unsigned char sb_get_flags(unsigned short objno);

/**
 * Set one or more communication (RAM) flags of a com-object. This does not
 * change already set flags of the com-object. See sb_unset_flags().
 * See SB_COMFLAG_ defines below.
 *
 * @param objno - the com-object to process
 * @param flags - the flags to set.
 */
void sb_set_flags(unsigned short objno, unsigned char flags);

/**
 * Unset one or more communication (RAM) flags of a com-object. Only the specified
 * flags are cleared. See sb_set_flags().
 * See SB_COMFLAG_ defines below.
 *
 * @param objno - the com-object to process
 * @param flags - the flags to clear. Use SB_COMFLAG_ALL to clear all flags.
 */
void sb_unset_flags(unsigned short objno, unsigned char flags);

/**
 * Process all com-objects. For every com-object that has the given flags set,
 * call the callback function. The flags are cleared during processing.
 *
 * @param flags - the communication (RAM) flags that must be set.
 * @param callback - pointer to the function to call when the flags are set.
 *                   The function gets the com-object number as an argument
 *
 * @example sb_process_flags(SB_COMFLAG_UPDATE, &comobj_updated);
 */
void sb_process_flags(unsigned char flags, void (*callback)(unsigned char));

/**
 * Get the configuration flags of a com-object.
 *
 * @param objno - the number of the com-object.
 * @return The object flags of the com-object.
 */
unsigned char sb_get_objflags(unsigned short objno);

/**
 * Process a multicast group telegram.
 *
 * This function is called by sb_process_tel(). It is usually not required to call
 * this function from within a user program. The telegram is stored in sbRecvTelegram[].
 *
 * @param destAddr - the destination group address.
 * @param apci - the APCI command of the telegram.
 */
void sb_process_group_tel(unsigned short destAddr, unsigned char apci);

/**
 * Send the next group value telegram. Does nothing if no group value
 * shall be sent.
 */
void sb_send_next_tel();


#ifdef SB_CUSTOM_READWRITE_VALUE
/**
 * Called when a write-value-request group telegram was received.
 * The value to be written is stored in sbRecvTelegram[].
 *
 * @param objno - the com-object to write the value.
 */
void sb_write_value_req(unsigned short objno);

/**
 * Called when a read-value-request group telegram was received.
 *
 * @param objno - the com-object to write the value.
 */
void sb_read_value_req(unsigned short objno);
#endif /*defined SB_CUSTOM_READWRITE_VALUE*/


/** Communication Flag: transmission status mask */
#define SB_COMFLAG_TRANS_MASK 0x3

/** Communication Flag: transmission status: idle/ok */
#define SB_COMFLAG_OK         0x0

/** Communication Flag: transmission status: idle/error */
#define SB_COMFLAG_ERROR      0x1

/** Communication Flag: transmission status: transmitting */
#define SB_COMFLAG_TRANS      0x2

/** Communication Flag: transmission status: transmission request */
#define SB_COMFLAG_TRANSREQ   0x3

/** Communication Flag: data request: 0=idle/response, 1=data request */
#define SB_COMFLAG_DATAREQ    0x4

/** Communication Flag: update: 0=not updated, 1=updated */
#define SB_COMFLAG_UPDATE     0x8

/** All communication flags. */
#define SB_COMFLAG_ALL        0x15


// Com object configuration flag: transmit + communication enabled
#define SB_COMOBJ_CONF_TRANS_COMM (SB_COMOBJ_CONF_COMM | SB_COMOBJ_CONF_TRANS)

// Com object configuration flag: read + communication enabled
#define SB_COMOBJ_CONF_READ_COMM (SB_COMOBJ_CONF_COMM | SB_COMOBJ_CONF_READ)

// Com object configuration flag: write + communication enabled
#define SB_COMOBJ_CONF_WRITE_COMM (SB_COMOBJ_CONF_COMM | SB_COMOBJ_CONF_WRITE)

#endif /*sb_comobj_h*/
