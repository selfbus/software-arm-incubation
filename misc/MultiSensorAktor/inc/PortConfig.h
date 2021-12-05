/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef PORTCONFIG_H_
#define PORTCONFIG_H_

enum PortType {
	PortTypeInputFloating,
	PortTypeInputPulldown,
	PortTypeInputPullup,
	PortTypeOutput,
	PortTypeDHT11,
	PortTypeDHT22,
	PortTypePWM
};

enum PortInAction
{
	PortInNothing,
	PortInOn,
	PortInOff,
	PortInToggle
};

enum PortOutBlink
{
	PortOutBlinkNever,
	PortOutBlinkIfOff,
	PortOutBlinkIfOn,
	PortOutBlinkAlways,
	PortOutBlinkObjAndOff,
	PortOutBlinkObjAndOn
};

#pragma pack(1)
struct PortConfig
{
	byte Type;
	unsigned short DHTPreFan;
	unsigned short DHTPreMeasure;
	unsigned short DHTDelay;
	unsigned short DHTOffsetDPT9;
	byte DebounceTime;
	byte InLo;
	byte InHi;
	byte InLoLong;
	byte InHiLong;
	bool InShortOnLong;
	unsigned short InLongTime;
	byte OutInvert;
	byte OutBlink;
	unsigned short OutBlinkOnTime;
	unsigned short OutBlinkOffTime;
	byte PWMInvert;
	byte Unused[6];
};
#pragma pack()

#endif /* PORTCONFIG_H_ */
