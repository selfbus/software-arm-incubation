/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef DEVICECONFIG_H_
#define DEVICECONFIG_H_

enum class BusSwitch : byte
{
	SPI0 = 0x01,
	SPI1 = 0x02,
	I2C = 0x04
};

inline bool operator&(BusSwitch a, BusSwitch b)
{
	return static_cast<bool>(static_cast<int>(a) & static_cast<int>(b));
}

#pragma pack(1)
struct DeviceConfig
{
	BusSwitch BusSwitches;
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
	byte SHT4xCount;
	byte SGP2xCount;
};
#pragma pack()

#endif /* DEVICECONFIG_H_ */
