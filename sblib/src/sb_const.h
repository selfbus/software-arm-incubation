/**
 * Constants
 */
#ifndef sb_const
#define sb_const


// Mask for ADC / memory application commands
#define SB_APCI_GROUP_MASK                  0x3c0

//
// Application commands (see KNX 3/3/7 p.8 Application Layer control field)
//

#define SB_GROUP_VALUE_READ_PDU             0x000
#define SB_GROUP_VALUE_WRITE_PDU            0x080
#define SB_INDIVIDUAL_ADDRESS_WRITE_PDU     0x0c0
#define SB_INDIVIDUAL_ADDRESS_READ_PDU      0x100
#define SB_INDIVIDUAL_ADDRESS_RESPONSE_PDU  0x140

#define SB_ADC_READ_PDU                     0x180
#define SB_ADC_RESPONSE_PDU                 0x1C0

#define SB_MEMORY_READ_PDU                  0x200
#define SB_MEMORY_RESPONSE_PDU              0x240
#define SB_MEMORY_WRITE_PDU                 0x280

#define SB_DEVICEDESCRIPTOR_READ_PDU        0x300
#define SB_DEVICEDESCRIPTOR_RESPONSE_PDU    0x340

#define SB_RESTART_PDU                      0x380

#define SB_AUTHORIZE_REQUEST_PDU            0x3d1
#define SB_AUTHORIZE_RESPONSE_PDU           0x3d2

#define SB_PROPERTY_VALUE_READ_PDU          0x3d5
#define SB_PROPERTY_VALUE_RESPONSE_PDU      0x3d6
#define SB_PROPERTY_VALUE_WRITE_PDU         0x3d7

//
// Transport commands
//

#define SB_T_CONNECT_PDU                    0x80
#define SB_T_DISCONNECT_PDU                 0x81
#define SB_T_ACK_PDU                        0xc2
#define SB_T_NACK_PDU                       0xc3


// TPCI (TPDU) commands

#define SB_GROUP_PDU                   0x00

// APDU commands
#define SB_READ_MASK_VERSION_REQUEST   0x00
#define SB_WRITE_GROUP_REQUEST         0x80
#define SB_READ_GROUP_REQUEST          0x00

/*
 * Com object configuration flags
 */

// Com object configuration flag: transmit enabled
#define SB_COMOBJ_CONF_TRANS 0x40

// Com object configuration flag: value memory type: 0=ram, 1=eeprom
#define SB_COMOBJ_CONF_VALUE_TYPE 0x20

// Com object configuration flag: write enabled
#define SB_COMOBJ_CONF_WRITE 0x10

// Com object configuration flag: read enabled
#define SB_COMOBJ_CONF_READ 0x08

// Com object configuration flag: communication enabled
#define SB_COMOBJ_CONF_COMM 0x04

// Com object configuration flag: transmission priority mask
#define SB_COMOBJ_CONF_PRIO_MASK 0x03

// Com object configuration flag: low transmission priority
#define SB_COMOBJ_CONF_PRIO_LOW 0x03

// Com object configuration flag: high transmission priority
#define SB_COMOBJ_CONF_PRIO_HIGH 0x02

// Com object configuration flag: alarm transmission priority
#define SB_COMOBJ_CONF_PRIO_ALARM 0x01

// Com object configuration flag: system transmission priority
#define SB_COMOBJ_CONF_PRIO_SYSTEM 0x00


// Commands for sending
#define SB_WRITE_VALUE_REQUEST         0x00000000
#define SB_READ_VALUE_RESPONSE         0x01000000
#define SB_OBJ_NCD_ACK                 0x81000000
#define SB_DEVICEDESCRIPTOR_RESPONSE   0x82000000
#define SB_INDIVIDUAL_ADDRESS_RESPONSE 0x83000000
#define SB_MEMORY_RESPONSE             0x84000000
#define SB_T_DISCONNECT                0x85000000
#define SB_ADC_RESPONSE                0x86000000
#define SB_AUTHORIZE_RESPONSE          0x87000000

// Mask for commands when sending
#define SB_SEND_CMD_MASK               0xff000000

// Mask for identifying unicast commands when sending
#define SB_SEND_UNICAST_CMD_MASK       0xf0000000

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

#define SB_INVALID_GRP_ADDRESS_IDX 0xFFFF
#define SB_ASSOC_ENTRY_SIZE        2

/*
#define SB_EEP_USERRAMADDRH    0x1C

#define SB_EEP_APPLICATION_RUN     (eeprom[RUNSTATE]==0xFF && !connected)&& !(status60&0x01)
*/
#endif /*sb_const*/
