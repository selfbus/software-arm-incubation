/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef PWMPINCONFIG_H_
#define PWMPINCONFIG_H_

enum PortFixedPwmValue : byte
{
	PortFixedPwmValueNever,
	PortFixedPwmValueOnLock = 128,
	PortFixedPwmValueAlways
};

#pragma pack(1)
struct PWMPinConfig
{
	bool Invert;
	PortFixedPwmValue FixedValueSwitch;
	byte FixedValueOnOn;
	byte FixedValueOnOff;
};
#pragma pack()

#endif /* PWMPINCONFIG_H_ */
