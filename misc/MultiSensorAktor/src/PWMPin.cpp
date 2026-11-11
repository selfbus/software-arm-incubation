/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/bcu_base.h>

#include <PWMPin.h>
#include <HelperFunctions.h>

//PWMPin *firstPwmPin = nullptr;
uint16_t currentPwmValue = 0;
/*extern "C" void TIMER16_0_IRQHandler()
{
	uint32_t now = millis();
	timer16_0.resetFlag(TIMER_MATCH_MAT0);
	currentPwmValue++;
	if (currentPwmValue == 2048)
	{
		currentPwmValue = 0;
	}

	IRQItem* item = firstPwmPin;
	while (item != nullptr)
	{
		item->Irq(now, currentPwmValue);
		item = item->nextPin;
	}
}*/

PWMPin::PWMPin(uint8_t firstComIndex, PWMPinConfig* config, GenericItem* parent, uint16_t& objRamPointer) : GenericPin(firstComIndex), config(config), parent(parent)
{
	comObjects->requestObjectRead(firstComIndex);
	comObjects->requestObjectRead(firstComIndex + 1);

	freqScaler = 0;
	freqDivider = (256 << freqScaler) - 1;

/*	if (firstPwmPin == nullptr)
	{
		setInterruptPriority(TIMER_16_0_IRQn, InterruptPriority::low);
		timer16_0.begin();
		timer16_0.prescaler(937);
		timer16_0.matchMode(TIMER_MATCH_MAT0, INTERRUPT | RESET);
		timer16_0.matchMode(TIMER_MATCH_MAT1, DISABLE);
		timer16_0.matchMode(TIMER_MATCH_MAT2, DISABLE);
		timer16_0.matchMode(TIMER_MATCH_MAT3, DISABLE);
		timer16_0.captureMode(TIMER_MATCH_MAT0, DISABLE);
		timer16_0.captureMode(TIMER_MATCH_MAT1, DISABLE);
		timer16_0.captureMode(TIMER_MATCH_MAT2, DISABLE);
		timer16_0.captureMode(TIMER_MATCH_MAT3, DISABLE);
		timer16_0.pwmDisable(TIMER_MATCH_MAT0);
		timer16_0.pwmDisable(TIMER_MATCH_MAT1);
		timer16_0.pwmDisable(TIMER_MATCH_MAT2);
		timer16_0.pwmDisable(TIMER_MATCH_MAT3);
		timer16_0.match(TIMER_MATCH_MAT0, 1);
		timer16_0.interrupts();
		timer16_0.start();
	}*/

	off = config->Invert != 0;
	on = config->Invert == 0;
//	nextPin = firstPwmPin;
//	firstPwmPin = this;

	HelperFunctions::setComObjPtr(comObjects, firstComIndex, BYTE_1, objRamPointer);
	HelperFunctions::setComObjPtr(comObjects, firstComIndex + 1, BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(comObjects, firstComIndex + 2, BIT_1, objRamPointer);
}

uint8_t PWMPin::GetState(uint32_t now, uint8_t updatedObjectNo)
{
	if (updatedObjectNo > 0 && (updatedObjectNo == firstComIndex || updatedObjectNo == firstComIndex + 1))
	{
		value = targetValue(*config, comObjects->objectRead(firstComIndex), comObjects->objectRead(firstComIndex + 1),
		                    value);

/*		if (value == 255)
		{
			value = 0xFFFF;
			parent->Irq(this, config->Invert == 0);
		}
		else if (value == 0)
		{
			parent->Irq(this, config->Invert != 0);
		}
		else
		{
			value <<= freqScaler;
		}*/
	}
	return 0;// currentState;
}

uint8_t PWMPin::targetValue(const PWMPinConfig& config, const uint8_t objectValue, const bool lock,
                            const uint8_t current)
{
	switch (config.FixedValueSwitch)
	{
	case PortFixedPwmValueNever:
		return objectValue;
	case PortFixedPwmValueOnLock:
		return lock ? objectValue : config.FixedValueOnOn;
	case PortFixedPwmValueAlways:
		return objectValue ? config.FixedValueOnOff : config.FixedValueOnOn;
	}
	return current;
}

/*void PWMPin::Irq(uint32_t now, uint16_t timerVal)
{
	if (value != 0xFFFF && value != 0)
	{
		timerVal &= freqDivider;
		if (timerVal == 0)
		{
			parent->Irq(this, on);
		}
		else if (timerVal == value)
		{
			parent->Irq(this, off);
		}
	}
}
*/
