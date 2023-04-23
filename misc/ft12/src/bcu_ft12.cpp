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
 * @file   bcu_ft12.cpp
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2022
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include "bcu_ft12.h"

BcuFt12::BcuFt12() : BcuFt12(new UserRamBCU1())
{
}

BcuFt12::BcuFt12(UserRamBCU1* userRamBcu1) :
        BcuBase(userRamBcu1, nullptr)
{
}

bool BcuFt12::processApci(ApciCommand apciCmd, unsigned char * telegram, uint8_t telLength, uint8_t * sendBuffer)
{
    return (false);
}

void BcuFt12::begin()
{
    userRam->status() = BCU_STATUS_LINK_LAYER | BCU_STATUS_TRANSPORT_LAYER | BCU_STATUS_APPLICATION_LAYER | BCU_STATUS_USER_MODE;
    userRam->runState() = 1;
    BcuBase::_begin();
}

bool BcuFt12::processBroadCastTelegram(ApciCommand apciCmd, unsigned char *telegram, uint8_t telLength)
{
    return (true);
}

bool BcuFt12::processGroupAddressTelegram(ApciCommand apciCmd, uint16_t groupAddress, unsigned char *telegram, uint8_t telLength)
{
    return (true);
}


/** @}*/
