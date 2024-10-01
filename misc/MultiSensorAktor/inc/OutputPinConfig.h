/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef OUTPUTPINCONFIG_H_
#define OUTPUTPINCONFIG_H_

enum class PortOutLockAction : byte
{
	PosNothing = 0,
	PosOff = 1,
	PosOn = 2,
	PosToggle = 3,
	NegNothing = 128,
	NegOff = 129,
	NegOn = 130,
	NegToggle = 131,
};

enum PortOutBlink : byte
{
	PortOutBlinkNever,
	PortOutBlinkIfOff,
	PortOutBlinkIfOn,
	PortOutBlinkAlways,
	PortOutBlinkObj,
	PortOutBlinkObjAndOff,
	PortOutBlinkObjAndOn,
	PortOutPulse,
};

#pragma pack(1)
struct OutputPinConfig
{
	bool Invert;
	PortOutBlink Blink;
	uint16_t BlinkOnTime;
	uint16_t BlinkOffTime;
	PortOutLockAction lockFlags;
};
#pragma pack()

#endif /* OUTPUTPINCONFIG_H_ */
