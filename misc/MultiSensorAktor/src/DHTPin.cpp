/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <DHTPin.h>
#include <HelperFunctions.h>

DHTPin::DHTPin(BcuBase* bcu, int port, byte firstComIndex, TempHumSensorConfig *config, bool dht11, uint16_t& objRamPointer) : GenericPin(bcu, firstComIndex), dht(DHT()), config(config), port(port), dht11(dht11)
{
	dht.DHTInit(port, dht11 ? DHT11 : DHT22);
	offset = config->Offset * 0.01f;

	HelperFunctions::setComObjPtr(bcu, firstComIndex, BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(bcu, firstComIndex + 1, BYTE_2, objRamPointer);
	HelperFunctions::setComObjPtr(bcu, firstComIndex + 2, BYTE_4, objRamPointer);
	HelperFunctions::setComObjPtr(bcu, firstComIndex + 3, BYTE_2, objRamPointer);
};

byte DHTPin::GetState(uint32_t now, byte updatedOjectNo)
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
			if (dht.readData(true))
			{
				float ftemp = dht.ConvertTemperature(CELCIUS) + offset;
				int16_t temp = (int16_t)(ftemp * 100);
				uint16_t hum = (uint16_t)(dht._lastHumidity * 100);
				bcu->comObjects->objectWriteFloat(firstComIndex + 1, temp);
				bcu->comObjects->objectWrite(firstComIndex + 2, (byte*)&ftemp);
				bcu->comObjects->objectWriteFloat(firstComIndex + 3, hum);
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
	return 0;
}
