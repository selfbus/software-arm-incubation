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


///todo this class exists also in USB-IF_Usb
class DeviceManagement
{
public:
  DeviceManagement(void);
  void DevMgnt_Tasks(void);
protected:
  unsigned int txtimeout;
  unsigned int rxtimeout;
  uint8_t LastDevSys;
};

extern DeviceManagement devicemgnt;
#endif /* DEVICE_MGNT_H_ */
