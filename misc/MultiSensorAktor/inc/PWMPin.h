/*
 * PWMPin.h
 *
 *  Created on: 27.12.2021
 *      Author: dridders
 */

#ifndef PWMPIN_H_
#define PWMPIN_H_

#include <sblib/eib/bcu_base.h>
#include <PWMPinConfig.h>
#include <GenericPin.h>
#include <GenericItem.h>

class IRQItem
{
public:
	virtual void Irq(uint32_t now, uint16_t timerVal) = 0;
	IRQItem *nextPin;
};

class PWMPin : public GenericPin, public IRQItem
{
public:
	PWMPin(BcuBase* bcu, byte firstComObj, PWMPinConfig *config, GenericItem* parent);
	~PWMPin() = default;

	byte GetState(uint32_t now, byte updatedObjectNo);

	int ConfigLength() { return sizeof(PWMPinConfig); }
	int ComObjCount() { return 3; }
	void Irq(uint32_t now, uint16_t timerVal);


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
