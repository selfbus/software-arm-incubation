/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef INPUTPINCONFIG_H_
#define INPUTPINCONFIG_H_

#include <sblib/eib/bcu_base.h>

enum PortInAction : byte
{
	PortInNothing = 0,
	PortInOff = 1,
	PortInOn = 2,
	PortInToggle = 3
};

// Values stored in ETS are deliberately 1..4 so that 0 can continue to mean
// "no action". The KNX DPT 2.001 payload is therefore action - 1.
enum PortInForcedAction : byte
{
	PortInForcedNothing = 0,
	PortInForcedNoPriorityOff = 1, // DPT 2.001 value 00
	PortInForcedNoPriorityOn = 2,  // DPT 2.001 value 01
	PortInForcedPriorityOff = 3,   // DPT 2.001 value 10
	PortInForcedPriorityOn = 4     // DPT 2.001 value 11
};

// In the KNX XML these flags are stored at BitOffset 2/3 of LockFlags.
// With the KNX parameter bit numbering this maps to 0x20/0x10.
enum PortInConfigFlag : byte
{
	PortInShortAction2Bit = 0x20,
	PortInLongAction2Bit = 0x10
};

#pragma pack(1)
// Keep this structure binary compatible with the existing PCA9555D input
// configuration. It is shared by ARM inputs and PCA9555D inputs.
struct InputPinConfig
{
	byte DebounceTime;
	byte Hi;
	byte Lo;
	byte HiLong;
	byte LoLong;
	bool ShortOnLong;
	uint16_t LongTime;
	byte LockFlags;
};

// Extra parameters used only for the native ARM input modes
// InputFloating/InputPullup/InputPulldown. They deliberately live outside
// InputPinConfig so that the PCA9555D configuration remains unchanged.
struct InputPinForcedConfig
{
	byte Hi;
	byte Lo;
	byte HiLong;
	byte LoLong;
};
#pragma pack()

#endif /* INPUTPINCONFIG_H_ */
