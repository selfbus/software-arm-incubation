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

FtControlField controlFieldFromByte(const uint8_t& controlByte)
{
    FtControlField cf;
    cf.fromBCUtoDevice = controlByte & 0x80;
    cf.isRequest = controlByte & 0x40;
    cf.frameCountBit = controlByte & 0x20;
    cf.frameCountBitValid = controlByte & 0x10;
    cf.functionCode = (FtFunctionCode)(controlByte & 0x0f);
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
        return (false);
    }

    if (frame[0] != FT_VARIABLE_START)
    {
        // start byte wrong
        return (false);
    }

    if (frame[0] != frame[3])
    {
        //4. bytes doesn't match start byte'
        return (false);
    }

    if (frame[1] != frame[2])
    {
        // mismatch of both length bytes
        return (false);
    }

    if (frame[frameLength - 1] != FT_END)
    {
        // end byte wrong
        return (false);
    }
    uint8_t userDataLength = frame[1];
    if (frameLength != (userDataLength + VARIABLE_FRAME_HEADER_LENGTH))
    {
        // length mismatch
        return (false);
    }

    if (calcCheckSum(frame, userDataLength) != frame[frameLength - 2])
    {
        return (false);
    }

    return (true);
}



/** @}*/
