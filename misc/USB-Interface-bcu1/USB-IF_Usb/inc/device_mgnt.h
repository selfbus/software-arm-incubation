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


///todo this class exists also in USB-IF_Knx
class DeviceManagement
{
public:
    DeviceManagement(void);
    void SysIf_Tasks(bool UsbActive);
    bool getKnxActive(void);
    DeviceMode getDeviceMode();

protected:
    uint32_t txtimeout = 0;
    uint32_t rxtimeout = 0;
    DeviceMode LastMode = DeviceMode::Halt;
    bool knxActive = false;

private:
    void setKnxActive(bool newValue);
};

extern DeviceManagement devicemgnt;

#endif /* DEVICE_MGNT_H_ */
