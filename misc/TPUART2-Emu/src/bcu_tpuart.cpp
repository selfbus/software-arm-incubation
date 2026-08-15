/*
 *  bcu_tpuart.cpp - Minimal BCU that turns the Selfbus library into a plain
 *                   KNX TP1 transceiver.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "bcu_tpuart.h"

BcuTpUart::BcuTpUart() : BcuTpUart(new UserRamBCU1())
{
}

BcuTpUart::BcuTpUart(UserRamBCU1* userRamBcu1) : BcuBase(userRamBcu1, nullptr)
{
}

void BcuTpUart::begin()
{
    // BCU_STATUS_TRANSPORT_LAYER is deliberately left cleared. Bus::handleTelegram()
    // then sets processTel unconditionally, which makes every telegram on the line
    // available in bus->telegram[] - exactly what a transceiver has to forward.
    //
    // BCU_STATUS_LINK_LAYER controls whether the library acknowledges received
    // frames on the bus. It is toggled at runtime for bus monitor and busy mode.
    userRam->status() = BCU_STATUS_LINK_LAYER | BCU_STATUS_APPLICATION_LAYER |
                        BCU_STATUS_SERIAL_PEI | BCU_STATUS_USER_MODE;
    userRam->runState() = 1;
    BcuBase::_begin();
}

void BcuTpUart::setLinkLayerActive(bool active)
{
    if (active)
    {
        userRam->status() |= (uint8_t)BCU_STATUS_LINK_LAYER;
    }
    else
    {
        userRam->status() &= (uint8_t)~BCU_STATUS_LINK_LAYER;
    }
}

bool BcuTpUart::linkLayerActive() const
{
    return ((userRam->status() & BCU_STATUS_LINK_LAYER) != 0);
}

bool BcuTpUart::processApci(ApciCommand apciCmd, unsigned char* telegram, uint8_t telLength,
                            uint8_t* sendBuffer)
{
    // A transceiver never answers on its own, the host does.
    return (false);
}

bool BcuTpUart::processGroupAddressTelegram(ApciCommand apciCmd, uint16_t groupAddress,
                                            unsigned char* telegram, uint8_t telLength)
{
    return (true);
}

bool BcuTpUart::processBroadCastTelegram(ApciCommand apciCmd, unsigned char* telegram,
                                         uint8_t telLength)
{
    return (true);
}
