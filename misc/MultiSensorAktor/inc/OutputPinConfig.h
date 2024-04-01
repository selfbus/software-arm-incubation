/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef OUTPUTPINCONFIG_H_
#define OUTPUTPINCONFIG_H_

enum class PortOutLockAction : byte
{
	Nothing,
	Off,
	On,
	Toggle
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
