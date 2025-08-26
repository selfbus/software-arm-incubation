/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef PWMPIN_H_
#define PWMPIN_H_

#include <sblib/eib/bcu_base.h>

#include <PWMPinConfig.h>
#include <GenericPin.h>
#include <GenericItem.h>

class PWMPin : public GenericPin
{
public:
	PWMPin(byte firstComObj, PWMPinConfig *config, GenericItem* parent, uint16_t& objRamPtr);
	~PWMPin() = default;

	byte GetState(uint32_t now, byte updatedObjectNo);

	int ConfigLength() { return sizeof(PWMPinConfig); }
	int ComObjCount() { return 3; }

protected:
	PWMPinConfig* config;
	uint16_t value = 0;
	GenericItem* parent;
	byte freqScaler;
	uint16_t freqDivider;
	byte on;
	byte off;
};

#endif
