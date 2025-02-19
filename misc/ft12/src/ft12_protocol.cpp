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
 * @file   ft12_protocol.cpp
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2022
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include "ft12_protocol.h"
#include "debug_handler.h"

uint8_t controlFieldToByte(const FtControlField& cf)
{
    uint8_t result = 0;
    if (cf.fromBCUtoDevice) // DIR bit 7 (physical transmission direction)
    {
        result |= 0x80;
    }

    if (cf.isRequest) // PRM bit 6 (primary message)
    {
        result |= 0x40;
    }
    if (cf.frameCountBitValid) // FCV bit 4 (frame count bit valid)
    {
        result |= 0x10;
        if (cf.frameCountBit) // FCB bit 5 (frame count bit)
        {
            result |= 0x20;
        }
    }
    result |= cf.functionCode & 0x0f; // bit 0-3 function code
    return result;
}

FtControlField controlFieldFromByte(const uint8_t& controlByte)
{
    FtControlField cf;
    cf.fromBCUtoDevice = controlByte & 0x80;    // DIR bit 7 (physical transmission direction)
    cf.isRequest = controlByte & 0x40;          // PRM bit 6 (primary message)
    cf.frameCountBit = controlByte & 0x20;      // FCB bit 5 (frame count bit)
    cf.frameCountBitValid = controlByte & 0x10; // FCV bit 4 (frame count bit valid)
    cf.functionCode = (FtFunctionCode)(controlByte & 0x0f); // bit 0-3 function code
    return (cf);
}

bool isValidFixedFrameHeader(const uint8_t* frame, uint8_t frameLength)
{
    // check length
    if (frameLength != FIXED_FRAME_LENGTH)
    {
        return (false);
    }

    // check start byte
    if (frame[0] != FT_FIXED_START)
    {
        return false;
    }

    // check end byte
    if (frame[3] != FT_END)
    {
        return false;
    }

    // check that control field equals checksum
    if (frame[1] != frame[2])
    {
        return false;
    }

    return (true);
}

uint8_t calcCheckSum(const uint8_t* frame, const uint8_t& userDataLength)
{
    // checksum = arithmetic sum modulo 256 over all user data bytes
    uint8_t checkSum = 0;
    for (uint8_t i = 0; i < userDataLength; i++)
    {
        checkSum += frame[4 + i];
    }
    return checkSum;
}

bool isValidVariableFrameHeader(const uint8_t* frame, uint8_t frameLength)
{
    if (frameLength < VARIABLE_FRAME_HEADER_LENGTH)
    {
        // to short
        debugFatal();
        return (false);
    }

    if (frame[0] != FT_VARIABLE_START)
    {
        // start byte wrong
        debugFatal();
        return (false);
    }

    if (frame[0] != frame[3])
    {
        //4. byte doesn't match start byte
        debugFatal();
        return (false);
    }

    if (frame[1] != frame[2])
    {
        // mismatch of both length bytes
        debugFatal();
        return (false);
    }

    if (frame[frameLength - 1] != FT_END)
    {
        // end byte wrong
        debugFatal();
        return (false);
    }

    uint8_t userDataLength = frame[1];
    if (frameLength != (userDataLength + VARIABLE_FRAME_HEADER_LENGTH))
    {
        // length mismatch
        if (frameLength > (userDataLength + VARIABLE_FRAME_HEADER_LENGTH))
        {
            // length now to long
            debugFatal();
            return false;
        }
        // length yet to short
        return false;
    }

    if (calcCheckSum(frame, userDataLength) != frame[frameLength - 2])
    {
        debugFatal();
        // checksum mismatch
        return (false);
    }

    return (true);
}



/** @}*/
