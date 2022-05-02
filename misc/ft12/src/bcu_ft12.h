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
 * @file   ft12_bcu.h
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2022
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#ifndef BCU_FT12_H_
#define BCU_FT12_H_

#include <sblib/eib/userRamBCU1.h>
#include <sblib/eib/bcu_base.h>
#include <sblib/eib/bus.h>

class BcuFt12: public BcuBase
{
public:
    BcuFt12();
    BcuFt12(UserRamBCU1* userRamBcu1);
    ~BcuFt12() = default;
    void begin();
    bool applicationRunning() const override {return (enabled);}

protected:
    unsigned char processApci(ApciCommand apciCmd, const uint16_t senderAddr, const int8_t senderSeqNo, bool *sendResponse, unsigned char * telegram, uint8_t telLength) override;
    bool processGroupAddressTelegram(ApciCommand apciCmd, uint16_t groupAddress, unsigned char *telegram, uint8_t telLength) override;
    bool processBroadCastTelegram(ApciCommand apciCmd, unsigned char *telegram, uint8_t telLength) override;
};



#endif /* BCU_FT12_H_ */
/** @}*/
