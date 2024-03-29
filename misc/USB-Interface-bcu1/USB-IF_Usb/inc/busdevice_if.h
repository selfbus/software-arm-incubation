/*
 *  busdevice_if.cpp - Pin configuration, system timer, activity LED
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef BUSDEVICE_IF_H_
#define BUSDEVICE_IF_H_

extern volatile unsigned int systemTime;

enum class TCdcDeviceMode
{
	Halt, //!< Initialization or USB stopped working
	HidOnly,
	UsbMon,
	BusMon,
	ProgBusChip,
	ProgUserChip
};

extern TCdcDeviceMode CdcDeviceMode;

class DeviceIf;

extern DeviceIf deviceIf;

class DeviceIf {
public:
  DeviceIf();
  bool Hid2Knx_Ena(void);

  void SystemInit(void);
  void PioInit(void);

  bool KnxSideProgMode(void);
  void DoActivityLed(bool LedEnabled);
  void BlinkActivityLed(void);
private:
  void SetActivityLed(bool onoff);
  bool enabled;
  int blinkcnt;
  int timecnt;
};

#endif /* BUSDEVICE_IF_H_ */
