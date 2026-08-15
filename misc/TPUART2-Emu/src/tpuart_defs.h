/*
 *  tpuart_defs.h - Service codes of the KNX TP-UART host protocol.
 *
 *  The values follow the Siemens TP-UART 2 (and ON Semi NCN512x) host
 *  interface description. Only the subset that is actually emulated is
 *  listed here, see README.md for the supported services.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef TPUART_DEFS_H_
#define TPUART_DEFS_H_

#include <stdint.h>

/**
 * Services sent by the host to the transceiver.
 */
enum TpUartHostService : uint8_t
{
    U_RESET_REQ             = 0x01, //!< reset the transceiver
    U_STATE_REQ             = 0x02, //!< request @ref U_STATE_IND
    U_SET_BUSY_REQ          = 0x03, //!< stop acknowledging received frames
    U_QUIT_BUSY_REQ         = 0x04, //!< resume acknowledging received frames
    U_BUSMON_REQ            = 0x05, //!< enter bus monitor mode (leave via reset)
    U_L_DATA_OFFSET_REQ     = 0x08, //!< 0x08..0x0C, offset = cmd & 0x07, unit 64 byte
    U_SYSTEM_STATE_REQ      = 0x0D, //!< request @ref U_SYSTEM_STAT_IND
    U_STOP_MODE_REQ         = 0x0E, //!< detach from the bus
    U_EXIT_STOP_MODE_REQ    = 0x0F, //!< re-attach to the bus
    U_ACK_INFORMATION_REQ   = 0x10, //!< 0x10..0x17, see @ref TpUartAckInformation
    U_CONFIGURE_REQ         = 0x18, //!< 0x18..0x1F, NCN512x only, ignored
    U_PRODUCT_ID_REQ        = 0x20, //!< product id request, ignored
    U_MXRSTCNT_REQ          = 0x24, //!< 0x24 + repetition byte, TP-UART 2 only
    U_SET_ADDRESS_REQ       = 0x28, //!< 0x28 + addrHigh + addrLow (used by knxd)
    U_INT_REG_WR_REQ        = 0x29, //!< 0x29..0x2B, NCN512x only, ignored
    U_INT_REG_RD_REQ        = 0x38, //!< 0x38..0x3B, NCN512x only, ignored
    U_L_DATA_END_REQ        = 0x40, //!< 0x40..0x7F, last byte of a frame
    U_L_DATA_START_CONT_REQ = 0x80, //!< 0x80..0xBF, byte n of a frame
    U_SET_ADDRESS_REQ_ALT   = 0xF1, //!< 0xF1 + addrHigh + addrLow (used by OpenKNX)
    U_SET_REPETITION_REQ    = 0xF2  //!< 0xF2 + repetition byte
};

/**
 * Flags of the @ref U_ACK_INFORMATION_REQ service.
 */
enum TpUartAckInformation : uint8_t
{
    U_ACK_REQ_ADDRESSED = 0x01, //!< the frame is addressed to the host
    U_ACK_REQ_BUSY      = 0x02, //!< answer with BUSY
    U_ACK_REQ_NACK      = 0x04  //!< answer with NACK
};

/**
 * Services sent by the transceiver to the host.
 */
enum TpUartControllerService : uint8_t
{
    L_DATA_STANDARD_IND = 0x90, //!< masked with @ref L_DATA_MASK
    L_DATA_EXTENDED_IND = 0x10, //!< masked with @ref L_DATA_MASK
    L_DATA_MASK         = 0xD3, //!< mask to detect an L_Data frame start
    L_DATA_CON          = 0x0B, //!< L_Data.con, or'ed with @ref L_DATA_CON_SUCCESS
    L_DATA_CON_SUCCESS  = 0x80, //!< positive confirmation
    U_RESET_IND         = 0x03, //!< answer to @ref U_RESET_REQ
    U_STATE_IND         = 0x07, //!< answer to @ref U_STATE_REQ, or'ed with error flags
    U_STOP_MODE_IND     = 0x2B, //!< answer to @ref U_STOP_MODE_REQ
    U_SYSTEM_STAT_IND   = 0x4B  //!< answer to @ref U_SYSTEM_STATE_REQ, plus one data byte
};

/**
 * Error flags reported in @ref U_STATE_IND.
 */
enum TpUartStateFlags : uint8_t
{
    TPUART_SLAVE_COLLISION     = 0x80,
    TPUART_RECEIVE_ERROR       = 0x40,
    TPUART_TRANSMIT_ERROR      = 0x20,
    TPUART_PROTOCOL_ERROR      = 0x10,
    TPUART_TEMPERATURE_WARNING = 0x08
};

/** Control byte mask to distinguish standard from extended L_Data frames. */
#define LPDU_FRAME_TYPE_MASK  (0xD3)
/** Control byte value of a standard L_Data frame after masking. */
#define LPDU_FRAME_TYPE_STD   (0x90)
/** Number of octets a standard L_Data frame has in addition to its payload. */
#define LPDU_STD_OVERHEAD     (8)

#endif /* TPUART_DEFS_H_ */
