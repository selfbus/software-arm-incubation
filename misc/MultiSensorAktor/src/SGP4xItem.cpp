/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <SGP4xItem.h>
#include <sblib/i2c.h>
#include <sblib/timer.h>
#include <HelperFunctions.h>

extern int portPins[32];

SGP4xItem::SGP4xItem(byte firstComIndex, TempHumSensorConfig *config, GenericItem* nextItem, uint16_t& objRamPointer) : GenericItem(firstComIndex, nextItem), config(config), /*sgp4x(SGP4xClass()),*/ nextAction(0)
{
//	sgp4x.init();

	BCU->comObjects->requestObjectRead(firstComIndex + 2);
	BCU->comObjects->requestObjectRead(firstComIndex + 3);

	HelperFunctions::setComObjPtr(BCU, firstComIndex, BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 1, BYTE_2, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 2, BYTE_2, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 3, BYTE_2, objRamPointer);
}

void SGP4xItem::Loop(uint32_t now, int updatedObjectNo)
{
	if (now >= nextAction || (now + (config->PreFan + config->PreMeasure + config->Delay) * 1000) < nextAction)
	{
		switch (state)
		{
		case 0:
			if (config->PreFan > 0)
			{
				BCU->comObjects->objectWrite(firstComIndex, 1);
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
				BCU->comObjects->objectWrite(firstComIndex, (int)0);
				nextAction = now + (config->PreMeasure * 1000);
			}
			else
			{
				nextAction = now;
			}
			state++;
			break;
		default:
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

