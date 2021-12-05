/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <PWMItem.h>
#include <sblib/digital_pin.h>


PWMItem *PWMItemFirstInstance;

PWMItem::PWMItem(BcuBase* bcu, int port, int firstComIndex, PortConfig *config, GenericItem* nextItem) : GenericItem(bcu, port, firstComIndex, config, nextItem)
{
	pinMode(port, OUTPUT);

	timer.begin();
	timer.prescaler(4800);
	timer.matchMode(MAT0, INTERRUPT);
	timer.matchMode(MAT1, RESET | INTERRUPT);
	timer.matchMode(MAT2, DISABLE);
	timer.matchMode(MAT3, DISABLE);
	timer.captureMode(MAT0, DISABLE);
	timer.captureMode(MAT1, DISABLE);
	timer.captureMode(MAT2, DISABLE);
	timer.captureMode(MAT3, DISABLE);
	timer.pwmDisable(MAT0);
	timer.pwmDisable(MAT1);
	timer.pwmDisable(MAT2);
	timer.pwmDisable(MAT3);
	timer.match(MAT1, 99);
	timer.interrupts();

	bcu->comObjects->requestObjectRead(firstComIndex);

	NextInstance = PWMItemFirstInstance;
	PWMItemFirstInstance = this;
}

void PWMItem::Loop(int updatedObjectNo)
{
	if (updatedObjectNo == firstComIndex)
	{
		int val = bcu->comObjects->objectRead(firstComIndex);
		if (!val)
		{
			timer.stop();
			digitalWrite(port, config->PWMInvert);
		}
		else if (val >= 100)
		{
			timer.stop();
			digitalWrite(port, !config->PWMInvert);
		}
		else
		{
			timer.match(MAT0, val);
			timer.start();
		}
	}
}

void PWMItem::IRQ()
{
	if (timer.flag(MAT0))
	{
		timer.resetFlag(MAT0);
		digitalWrite(port, !config->PWMInvert);
	}
	if (timer.flag(MAT1))
	{
		timer.resetFlag(MAT1);
		digitalWrite(port, config->PWMInvert);
	}
}


extern "C" void TIMER16_0_IRQHandler()
{
	PWMItem *item = PWMItemFirstInstance;
	while (item != nullptr)
	{
		item->IRQ();
		item = item->NextInstance;
	}
}
