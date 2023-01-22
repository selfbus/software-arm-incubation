/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <PWMPin.h>
#include <HelperFunctions.h>

PWMPin *firstPwmPin = nullptr;
uint16_t currentPwmValue = 0;
extern "C" void TIMER16_0_IRQHandler()
{
	uint32_t now = millis();
	timer16_0.resetFlag(MAT0);
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
}

PWMPin::PWMPin(BcuBase* bcu, byte firstComIndex, PWMPinConfig* config, GenericItem* parent, uint16_t& objRamPointer) : GenericPin(bcu, firstComIndex), config(config), parent(parent)
{
	bcu->comObjects->requestObjectRead(firstComIndex);
	bcu->comObjects->requestObjectRead(firstComIndex + 1);

	freqScaler = 0;
	freqDivider = (256 << freqScaler) - 1;

/*	if (firstPwmPin == nullptr)
	{
		NVIC_SetPriority(TIMER_16_0_IRQn, 3);
		timer16_0.begin();
		timer16_0.prescaler(937);
		timer16_0.matchMode(MAT0, INTERRUPT | RESET);
		timer16_0.matchMode(MAT1, DISABLE);
		timer16_0.matchMode(MAT2, DISABLE);
		timer16_0.matchMode(MAT3, DISABLE);
		timer16_0.captureMode(MAT0, DISABLE);
		timer16_0.captureMode(MAT1, DISABLE);
		timer16_0.captureMode(MAT2, DISABLE);
		timer16_0.captureMode(MAT3, DISABLE);
		timer16_0.pwmDisable(MAT0);
		timer16_0.pwmDisable(MAT1);
		timer16_0.pwmDisable(MAT2);
		timer16_0.pwmDisable(MAT3);
		timer16_0.match(MAT0, 1);
		timer16_0.interrupts();
		timer16_0.start();
	}*/

	off = config->Invert != 0;
	on = config->Invert == 0;
	nextPin = firstPwmPin;
	firstPwmPin = this;

	HelperFunctions::setComObjPtr(bcu, firstComIndex, BYTE_1, objRamPointer);
	HelperFunctions::setComObjPtr(bcu, firstComIndex + 1, BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(bcu, firstComIndex + 2, BIT_1, objRamPointer);
}

byte PWMPin::GetState(uint32_t now, byte updatedObjectNo)
{
	if (updatedObjectNo > 0 && (updatedObjectNo == firstComIndex || updatedObjectNo == firstComIndex + 1))
	{
		switch (config->FixedValueSwitch)
		{
		case PortFixedPwmValueNever:
			value = bcu->comObjects->objectRead(firstComIndex);
			break;
		case PortFixedPwmValueOnLock:
			if (bcu->comObjects->objectRead(firstComIndex + 1))
			{
				value = bcu->comObjects->objectRead(firstComIndex);
			}
			else
			{
				value = config->FixedValueOnOn;
			}
			break;
		case PortFixedPwmValueAlways:
			if (bcu->comObjects->objectRead(firstComIndex))
			{
				value = config->FixedValueOnOff;
			}
			else
			{
				value = config->FixedValueOnOn;
			}
			break;
		}

		if (value == 255)
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
		}
	}
	return 0;// currentState;
}

void PWMPin::Irq(uint32_t now, uint16_t timerVal)
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
