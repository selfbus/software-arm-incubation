/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <DHTItem.h>

DHTItem::DHTItem(BcuBase* bcu, int port, int firstComIndex, PortConfig *config, GenericItem* nextItem) : GenericItem(bcu, port, firstComIndex, config, nextItem), dht(DHT())
{
	int type = DHT11;
	if (config->Type == PortTypeDHT22)
	{
		type = DHT22;
	}
	dht.DHTInit(port, type);
	offset = dptFromFloat(config->DHTOffsetDPT9) * 0.01;
};

void DHTItem::Loop(int updatedOjectNo)
{
	unsigned int now = millis();
	if (now > nextAction)
	{
		switch (state)
		{
		case 0:
			if (config->DHTPreFan > 0)
			{
				bcu->comObjects->objectWrite(firstComIndex + 0, 1);
				nextAction = now + (config->DHTPreFan * 1000);
			}
			else
			{
				nextAction = now;
			}
			state++;
			break;
		case 1:
			if (config->DHTPreFan > 0)
			{
				bcu->comObjects->objectWrite(firstComIndex + 0, (int)0);
				nextAction = now + (config->DHTPreMeasure * 1000);
			}
			else
			{
				nextAction = now;
			}
			state++;
			break;
		case 2:
			if (dht.readData(false))
			{
				float ftemp = dht.ConvertTemperature(CELCIUS) + offset;
				int16_t temp = (int16_t)(ftemp * 100);
				uint16_t hum = (uint16_t)(dht._lastHumidity * 100);
				bcu->comObjects->objectWriteFloat(firstComIndex + 1, temp);
				bcu->comObjects->objectWrite(firstComIndex + 2, (byte*)&ftemp);
				bcu->comObjects->objectWriteFloat(firstComIndex + 3, hum);
				nextAction = now + (config->DHTDelay * 1000);
			}
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
