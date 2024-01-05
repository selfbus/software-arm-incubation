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

/*class IRQItem
{
public:
	virtual void Irq(uint32_t now, uint16_t timerVal) = 0;
	IRQItem *nextPin;
};*/

class PWMPin : public GenericPin //, public IRQItem
{
public:
	PWMPin(byte firstComObj, PWMPinConfig *config, GenericItem* parent, uint16_t& objRamPtr);
	~PWMPin() = default;

	byte GetState(uint32_t now, byte updatedObjectNo);

	int ConfigLength() { return sizeof(PWMPinConfig); }
	int ComObjCount() { return 3; }
//	void Irq(uint32_t now, uint16_t timerVal);


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
