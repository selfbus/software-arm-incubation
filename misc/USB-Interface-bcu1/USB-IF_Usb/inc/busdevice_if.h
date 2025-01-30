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
    ///\todo verify docu
	Halt,           //!< Initialization or USB stopped working
	HidOnly,        //!< KNX-Interface only, KNX bus interface for ETS, knxd or Updater.
	UsbMon,         //!< USB-Monitor, sends raw EMI 1 messages to virtual serial port of the USB port.
	BusMon,         //!< KNX-Busmonitor, sends decoded KNX TP1 telegrams to virtual serial port of the USB port.

	/**
     * ISP-Programmer for KNX bus access mcu (TS_ARM connected at P1).
     *
     * @note Jumper JP5 (PIO 1_19) must be closed on startup.
     */
	ProgBusChip,

    /**
     * ISP-Programmer for user mcu (connected at Prog-If (P3)).
     *
     * @note Jumper JP2 (PIO 0_3) and JP6 (3.3V)  must be closed.
     */
	ProgUserChip    //!< ISP-Programmer for ARM controller
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
