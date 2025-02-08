/*
 *  device_mgnt.h - Device management, mode management and mode switching
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef DEVICE_MGNT_H_
#define DEVICE_MGNT_H_

#include "device_mgnt_const.h"
#include "prog_uart.h"
#include "emi_knx.h"

///todo this class exists also in USB-IF_Usb
class DeviceManagement
{
public:
    DeviceManagement(void) = delete;
    DeviceManagement(ProgUart * softUART, EmiKnxIf * emiKnxIf);
    void DevMgnt_Tasks(void);
protected:
    uint32_t txtimeout = 0;
    uint32_t rxtimeout = 0;
    DeviceMode deviceMode = DeviceMode::Invalid;

private:
    ProgUart * softUART = nullptr;
    EmiKnxIf * emiKnxIf = nullptr;
    void setDeviceMode(DeviceMode newDeviceMode);

};

#endif /* DEVICE_MGNT_H_ */
