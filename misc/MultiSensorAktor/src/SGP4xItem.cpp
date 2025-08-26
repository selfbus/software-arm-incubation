/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/bcu_base.h>
#include <sblib/i2c.h>
#include <sblib/timer.h>

#include <SGP4xItem.h>
#include <HelperFunctions.h>

extern int portPins[32];

SGP4xItem::SGP4xItem(byte firstComIndex, TempHumSensorConfig *config, GenericItem* nextItem, uint16_t& objRamPointer) : GenericItem(firstComIndex, nextItem), config(config), /*sgp4x(SGP4xClass()),*/ nextAction(0)
{
    ///\todo implement sgp4x functionality
//    sgp4x.init(GasIndexAlgorithm_DEFAULT_SAMPLING_INTERVAL * 1000);

	HelperFunctions::setComObjPtr(BCU, firstComIndex, BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 1, BYTE_2, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 2, BYTE_2, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 3, BYTE_2, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 4, BYTE_2, objRamPointer);

	BCU->comObjects->requestObjectRead(firstComIndex + 2);
	BCU->comObjects->requestObjectRead(firstComIndex + 3);
}

void SGP4xItem::Loop(uint32_t now, int updatedObjectNo)
{
	if (updatedObjectNo == firstComIndex + 3 || updatedObjectNo == firstComIndex + 4)
	{
		temp = BCU->comObjects->objectReadFloat(firstComIndex + 2) * 0.01f;
		hum = BCU->comObjects->objectReadFloat(firstComIndex + 3) * 0.01f;
	}

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
/*			if (sgp4x.measureRawSignal(hum, temp, temp != -100 && hum != -100) == SGP4xResult::success)
			{
				BCU->comObjects->objectWrite(firstComIndex + 1, sgp4x.getVocIndexValue());
				BCU->comObjects->objectWrite(firstComIndex + 4, sgp4x.getNoxIndexValue());
			}*/
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

