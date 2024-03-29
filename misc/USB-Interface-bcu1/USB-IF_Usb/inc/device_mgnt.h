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

#define C_Dev_Idle 1
#define C_Dev_Sys  2
#define C_DevSys_Disable 1
#define C_DevSys_Normal 2
#define C_DevSys_CdcMon 3
#define C_DevSys_UsrPrg 4
#define C_Dev_Isp  3

//#define C_TxTimeout 450
#define C_RxTimeout 450
#define C_IdlePeriod 200

class DeviceManagement
{
public:
	DeviceManagement(void);
	void SysIf_Tasks(bool UsbActive);
	bool KnxIsActive(void);
protected:
  unsigned int txtimeout;
  unsigned int rxtimeout;
  TCdcDeviceMode LastMode;
  bool KnxActive;
};

extern DeviceManagement devicemgnt;

#endif /* DEVICE_MGNT_H_ */
