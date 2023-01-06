/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <CCS811Item.h>
#include <sblib/i2c.h>
#include <sblib/timer.h>

extern int portPins[32];

CCS811Item::CCS811Item(BcuBase* bcu, byte firstComIndex, CCS811Config *config, GenericItem* nextItem) : GenericItem(bcu, firstComIndex, nextItem), config(config), ccs811(CCS811Class()), nextAction(0)
{
	for (int i = 0; i < 10 && !configured; i++)
	{
		delay(i * 1000);
		configured = ccs811.begin(config->addr | 0x5A, portPins[config->wakePin]);
	}
	bcu->comObjects->requestObjectRead(firstComIndex + 3);
	bcu->comObjects->requestObjectRead(firstComIndex + 4);
}

void CCS811Item::Loop(uint32_t now, int updatedObjectNo)
{
	if (updatedObjectNo == firstComIndex + 3 || updatedObjectNo == firstComIndex + 4)
	{
		float t = bcu->comObjects->objectReadFloat(firstComIndex + 3) * 0.01f;
		float rh = bcu->comObjects->objectReadFloat(firstComIndex + 4) * 0.01f;

		ccs811.compensate(t, rh);
	}

	if (now >= nextAction || (now + (config->PreFan + config->PreMeasure + config->Delay) * 1000) < nextAction)
	{
		switch (state)
		{
		case 0:
			if (config->PreFan > 0)
			{
				bcu->comObjects->objectWrite(firstComIndex + 0, 1);
				nextAction = now + (config->PreFan * 1000);
			}
			else
			{
				nextAction = now;
			}
			state++;
			break;
		case 1:
			if (config->PreFan > 0)
			{
				bcu->comObjects->objectWrite(firstComIndex + 0, (int)0);
				nextAction = now + (config->PreMeasure * 1000);
			}
			else
			{
				nextAction = now;
			}
			state++;
			break;
		case 2:
			if (!configured)
			{
				configured = ccs811.begin(config->addr | 0x5A, portPins[config->wakePin]);
				if (baseline != 0)
				{
					ccs811.setBaseline(baseline);
				}
			}

			if (configured)
			{
				configured = ccs811.getData();
				if (configured)
				{
					baseline = ccs811.getBaseline();
					bcu->comObjects->objectWrite(firstComIndex + 1, ccs811.CO2);
					bcu->comObjects->objectWrite(firstComIndex + 2, ccs811.TVOC);
				}
			}

			nextAction = now + (config->Delay * 1000);
			state = 0;
			break;
		}

		// Überlauf würde zu Dauermessung führen bis SystemTime auch überläuft
		if (nextAction < now)
		{
			nextAction = 0xFFFFFFFF;
		}
	}
}

