/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef DEVICECONFIG_H_
#define DEVICECONFIG_H_

enum BusSwitch
{
	BusSwitchSPI0 = 0x01,
	BusSwitchSPI1 = 0x02,
	BusSwitchI2C = 0x04
};

#pragma pack(1)
struct DeviceConfig
{
	byte BusSwitches;
	byte SCK0;
	byte SCK1;
	byte MISO1;
	byte BH1750Count;
	byte CCS811Count;
	byte DS3231Count;
	byte iAQCoreCount;
	byte SHT2xCount;
	byte PCA9555DCount;
	uint8_t PortAssignment[32];
};
#pragma pack()

#endif /* DEVICECONFIG_H_ */
