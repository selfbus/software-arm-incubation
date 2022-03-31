/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <SHT2xItem.h>
#include <sblib/i2c.h>
#include <sblib/timer.h>

extern int portPins[32];

SHT2xItem::SHT2xItem(BcuBase* bcu, byte firstComIndex, SHT2xConfig *config, GenericItem* nextItem) : GenericItem(bcu, firstComIndex, nextItem), config(config), sht2x(SHT2xClass()), nextAction(0)
{
	sht2x.Init();
	offset = config->Offset * 0.01;
}

void SHT2xItem::Loop(uint32_t now, int updatedObjectNo)
{
	if (now >= nextAction || (now + (config->PreFan + config->PreMeasure + config->Delay) * 1000) < nextAction)
	{
		switch (state)
		{
		case 0:
			if (config->PreFan > 0)
			{
				bcu->comObjects->objectWrite(firstComIndex, 1);
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
				bcu->comObjects->objectWrite(firstComIndex, (int)0);
				nextAction = now + (config->PreMeasure * 1000);
			}
			else
			{
				nextAction = now;
			}
			state++;
			break;
		default:
			int16_t temp = (int16_t)(sht2x.GetTemperature() * 100);
			float ftemp = temp * 0.01f + offset;
			uint16_t hum = (uint16_t)(sht2x.GetHumidity());
			bcu->comObjects->objectWriteFloat(firstComIndex + 1, temp);
			bcu->comObjects->objectWrite(firstComIndex + 2, (byte*)&ftemp);
			bcu->comObjects->objectWriteFloat(firstComIndex + 3, hum);
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
	return;
}

