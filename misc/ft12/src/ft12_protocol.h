/**************************************************************************//**
 * @addtogroup SBLIB_MAIN_GROUP Main Group Description
 * @defgroup SBLIB_SUB_GROUP_1 Sub Group 1 Description
 * @ingroup SBLIB_MAIN_GROUP
 * @brief   
 * @details 
 *
 *
 * @{
 *
 * @file   ft12_protocol.h
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2022
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#ifndef FT12_PROTOCOL_H_
#define FT12_PROTOCOL_H_

#include <stdint.h>

#define FT12_EXCHANGE_TIMEOUT_BITS (510)  //!< Time-out for end of exchange in case of SEND/CONFIRM or REQUEST/RESPOND (KNX Spec. 2.1 3/6/2 6.4.8 p.29)
#define FT12_REPEAT_LIMIT (3)             //!< Repeat limit the retransmissions due to transmission errors (KNX Spec. 2.1 3/6/2 6.4.8 p.29)
#define FT12_LINE_IDLE_TIMEOUT_BITS (33)  //!< maximum bit-time between two characters, minimum line idle time before an error is detected

#define FIXED_FRAME_LENGTH (4)            //!< Length of a fixed ft12 frame
#define VARIABLE_FRAME_HEADER_LENGTH (6)  //!< Header length of a variable ft12 frame

constexpr int16_t InvalidCheckSum = -1;  //!< Invalid checksum

enum class FtError
{
    FT_NO_ERROR = 0,
    FT_TOO_SHORT = 1,
    FT_TOO_LONG = 2,
    FT_INVALID_START = 3,
    FT_INVALID_END = 4,
    FT_INVALID_CHECKSUM = 5,
    FT_INVALID_LENGTH = 6
};

/**
 * FT frame type
 * @note KNX Spec. 2.1 3/6/2 6.4.3.2 p.23ff
 */
enum FtFrameType
{
    FT_NONE           = 0x00, //!< none / unspecified
    FT_FIXED_START    = 0x10, //!< start byte of a frame with fixed length
    FT_END            = 0x16, //!< end byte of a frame with fixed length
    FT_VARIABLE_START = 0x68, //!< start byte of a frame with variable length
    FT_ACK            = 0xe5, //!< positive acknowledgment
};

/**
 * FT control field code
 * @note KNX Spec. 2.1 3/6/2 6.4.4 p.25ff
 */
enum FtFunctionCode
{
    FC_SEND_RESET = 0x00,  //!< Reset the remote link
    FC_SEND_UDAT  = 0x03,  //!< User data
    FC_REQ_STATUS = 0x09,  //!< Request status of link
};

/**
 * Control field
 * @note KNX Spec. 2.1 3/6/2 6.4.4 control field p.25ff
 */
struct FtControlField
{
    bool fromBCUtoDevice;        //!< true if transmission is from BCU to device, false if opposite direction
    bool isRequest;              //!< true if message is from primary (initiating station), otherwise false
    bool frameCountBit;          //!< alternating transmission bit for SEND/CONFIRM transmission
    bool frameCountBitValid;     //!< alternating bit for SEND/CONFIRM
    FtFunctionCode functionCode; //!< ft12 function code e.g. @ref FC_SEND_RESET
};


/**
 * External message interface codes
 * @details - DLL = Data Link Layer
 *          - NL  = Network Link Layer
 *          - TL  = Transport Layer
 *          - TLG = Transport Layer Group oriented
 *          - TLC = Transport Layer local
 *          - AL  = Application Layer
 *          - ALG = Application Layer Group oriented
 *          - MAN = Application Layer Management part
 *          - PEI = Physical External Interface
 *          - USR = Application running in BCU, if User not running message directed to PEI
 *
 * @note KNX Spec. 2.1 3/6/3 2 Message format p. 6ff
 */
enum EmiCode
{
    PH_Data_Req_B = 0x01,
    PH_Data_Con_B = 0x1e,
    PH_Data_Ind_B = 0x19,

    // Link layer
    L_Busmon_Ind    = 0x2B,     //!< -> PEI

    L_Data_Req      = 0x11,     //!< -> DLL, Send data to the bus
    L_Data_Con      = 0x2E,     //!< -> NL
    L_Data_Ind      = 0x29,     //!< -> NL, Data from the bus

    L_SystemBroadcast_Req = 0x17,
    L_SystemBroadcast_Con = 0x26,
    L_SystemBroadcast_Ind = 0x28,

    L_Plain_Data_Req = 0x10,    //!< -> DLL

    L_PollData_Req  = 0x13,     //!< -> NL
    L_PollData_Con = 0x25,      //!< -> NL

    L_Meter_Ind = 0x24,

    // Network layer
    N_Data_Individual_Req = 0x21, //!< -> NL
    N_Data_Individual_Con = 0x4e, //!< -> TLC
    N_Data_Individual_Ind = 0x49, //!< -> TLC

    N_Data_Group_Req = 0x22, //!< -> NL
    N_Data_Group_Con = 0x3e, //!< -> TLG
    N_Data_Group_Ind = 0x3a, //!< -> TLG

    N_Data_Broadcast_Req = 0x2c, //!< -> NL
    N_Data_Broadcast_Con = 0x4f, //!< -> TLC
    N_Data_Broadcast_Ind = 0x4d, //!< -> TLC

    N_Data_Poll_Req = 0x23, //!< -> NL
    N_Data_Poll_Con = 0x35, //!< -> TLG

    // Transport layer
    T_Connect_Req = 0x43, //!< -> TLC, Connect request
    T_Connect_Con = 0x86, //!< -> MAN, Connect reply
    T_Connect_Ind = 0x85, //!< -> MAN

    T_Disconnect_Req = 0x44, //!< -> TLC
    T_Disconnect_Con = 0x88, //!< -> MAN
    T_Disconnect_Ind = 0x87, //!< -> MAN

    T_Data_Connected_Req = 0x41, //!< -> TLC, Connected data request
    T_Data_Connected_Con = 0x8e, //!< -> MAN, Connected data reply
    T_Data_Connected_Ind = 0x89, //!< -> MAN

    T_Data_Group_Req = 0x32, //!< -> TLG
    T_Data_Group_Con = 0x7e, //!< -> ALG
    T_Data_Group_Ind = 0x7a, //!< -> ALG

    T_Data_Broadcast_Req = 0x4c, //!< -> TLC
    T_Data_Broadcast_Con = 0x8f, //!< -> MAN
    T_Data_Broadcast_Ind = 0x8d, //!< -> MAN

    T_Data_Individual_Req = 0x4a, //!< -> TLC
    T_Data_Individual_Con = 0x9c, //!< -> MAN
    T_Data_Individual_Ind = 0x94, //!< -> MAN

    T_Poll_Data_Req = 0x33, //!< -> TLG
    T_Poll_Data_Con = 0x75, //!< -> ALG

    M_Connect_Ind = 0xd5, //!< -> User, or PEI if user not running

    M_User_Data_Connected_Req = 0x82, //!< -> MAN
    M_User_Data_Connected_Con = 0xd1, //!< -> User, or PEI if user not running
    M_User_Data_Connected_Ind = 0xd2, //!< -> User, or PEI if user not running

    A_Data_Group_Req = 0x72, //!< -> ALG
    A_Data_Group_Con = 0xee, //!< -> User, or PEI if user not running
    A_Data_Group_Ind = 0xea, //!< -> User, or PEI if user not running

    M_User_Data_Individual_Req = 0x81, //!< -> MAN
    M_User_Data_Individual_Con = 0xde, //!< -> User, or PEI if user not running
    M_User_Data_Individual_Ind = 0xd9, //!< -> User, or PEI if user not running

    A_Poll_Data_Req = 0x73, //!< -> ALG
    A_Poll_Data_Con = 0xe5, //!< -> User, or PEI if user not running

    M_InterfaceObj_Data_Req = 0x9a, //!< -> MAN
    M_InterfaceObj_Data_Con = 0xdc, //!< -> User, or PEI if user not running
    M_InterfaceObj_Data_Ind = 0xd4, //!< -> User, or PEI if user not running

    U_Value_Read_Req = 0x74, //!< -> ALG
    U_Value_Read_Con = 0xe4, //!< -> User
    U_Value_Read_Ind = 0x7c, //!< -> ALG

    U_Flags_Read_Req = 0x7c, //!< -> ALG
    U_Flags_Read_Con = 0xec, //!< -> User

    U_Event_Ind = 0xe7, //!< -> User

    U_Value_Write_Req = 0x71, //!< ->ALG

    U_User_Data_0 = 0xd0, //!< -> User, Spec. says > 0xd0. Does it mean range 0xd0 0xff ?

    PC_Set_Value_Req = 0xa6, //!< -> MAN

    PC_Get_Value_Req = 0xac, //!< -> MAN
    PC_Get_Value_Con = 0xab, //!< -> PEI

    PEI_Identify_Req = 0xa7, //!<         PEI identify request
    PEI_Identify_Con = 0xa8, //!< -> PEI, PEI identify reply
    PEI_Switch_Req = 0xa9,   //!<         Switch the PEI mode

    TM_Timer_Ind = 0xc1, //!< ->User
};

/**
 * Converts a @ref FtControlField struct to its corresponding byte representation.
 *
 * @param cf A `FtControlField` structure representing the control field to convert.
 *         - `cf.fromBCUtoDevice`: Sets bit 7 (DIR), representing the physical transmission direction.
 *         - `cf.isRequest`:       Sets bit 6 (PRM), indicating a primary message.
 *         - `cf.frameCountBitValid`: Sets bit 4 (FCV), indicating if the frame count bit is valid.
 *         - `cf.frameCountBit`:  If `frameCountBitValid` is true, sets bit 5 (FCB), the frame count bit.
 *         - `cf.functionCode`:   Sets bits 0-3, representing the function code.
 *
 * @return The resulting byte value.
 */
uint8_t controlFieldToByte(const FtControlField& cf);

/**
 * Converts a byte into a @ref FtControlField
 *
 * @param controlByte - The byte to convert into a @ref FtControlField
 * @return The converted @ref FtControlField
 */
FtControlField controlFieldFromByte(const uint8_t& controlByte);

FtError isValidFixedFrameHeader(const uint8_t* frame, uint8_t frameLength);
FtError isValidVariableFrameHeader(const uint8_t* frame, uint8_t frameLength);
uint8_t calcCheckSum(const uint8_t* frame, const uint8_t& userDataLength);

#endif /* FT12_PROTOCOL_H_ */
/** @}*/
