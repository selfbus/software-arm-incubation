/**
 * Constants
 */
#ifndef sb_const
#define sb_const


// TPCI (TPDU) commands
#define SB_GROUP_PDU                   0x00
#define SB_BROADCAST_PDU_SET_PA_REQ    0x00
#define SB_BROADCAST_PDU_READ_PA       0x01
#define SB_DATA_PDU_MEMORY_OPERATIONS  0x42
#define SB_DATA_PDU_MISC_OPERATIONS    0x43
#define SB_CONNECT_PDU                 0x80
#define SB_DISCONNECT_PDU              0x81
#define SB_NACK_PDU                    0x83

// APDU commands
#define SB_SET_PHYSADDR_REQUEST        0XC0
#define SB_READ_PHYSADDR_REQUEST       0X00
#define SB_WRITE_MEMORY_REQUEST        0x80
#define SB_READ_MEMORY_REQUEST         0x00
#define SB_RESTART_REQUEST             0x80
#define SB_READ_MASK_VERSION_REQUEST   0x00
#define SB_WRITE_GROUP                 0x80
#define SB_READ_GROUP_REQUEST          0x00

// pseudo-objects for unicast sending
#define SB_OBJ_NCD_ACK                 129

/**
 * Bus short acknowledgment frame: acknowledged
 */
#define SB_BUS_ACK 0xcc

/**
 * Bus short acknowledgment frame: not acknowledged
 */
#define SB_BUS_NACK 0x0c

/**
 * Bus short acknowledgment frame: busy
 */
#define SB_BUS_BUSY 0xc0

/**
 * Bus short acknowledgment frame: not acknowledged & busy
 * Shall be handled as SB_BUS_BUSY
 */
#define SB_BUS_NACK_BUSY 0x00

/**
 * EEPROM Addresses
 */
#define SB_EEP_RUNSTATE        0x0D    // run-state (0x00=stop, 0xFF=run)
#define SB_EEP_ASSOCTABPTR     0x11    // address for the pointer to the association table
#define SB_EEP_COMMSTABPTR     0x12    // address for the pointer to the communication object table

#define SB_EEP_ADDRTAB         0x16    // base address of the association table

/*
#define SB_EEP_USERRAMADDRH    0x1C

#define SB_EEP_APPLICATION_RUN     (eeprom[RUNSTATE]==0xFF && !connected)&& !(status60&0x01)
*/
#endif /*sb_const*/
