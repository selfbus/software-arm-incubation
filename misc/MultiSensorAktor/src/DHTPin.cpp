/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/bcu_base.h>

#include <DHTPin.h>
#include <HelperFunctions.h>
#include <ARMPinItem.h>


DHTPin::DHTPin(int port, byte firstComIndex, DHTConfig *config, bool dht11, uint16_t& objRamPointer) : GenericPin(firstComIndex), dht(DHT()), config(config)
{
	if (config->PowerPin != 99)
	{
		pinMode(ARMPinItem::PortPins[config->PowerPin], OUTPUT);
		digitalWrite(ARMPinItem::PortPins[config->PowerPin], false);
	}

	dht.DHTInit(port, dht11 ? DHT11 : DHT22);

	HelperFunctions::setComObjPtr(BCU, firstComIndex, BIT_1, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 1, BYTE_2, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 2, BYTE_4, objRamPointer);
	HelperFunctions::setComObjPtr(BCU, firstComIndex + 3, BYTE_2, objRamPointer);

	retries = 3;
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
		case 2:
			if (config->PowerPin != 99)
			{
				digitalWrite(ARMPinItem::PortPins[config->PowerPin], true);
			}
			nextAction = now + 2000;
			state++;
			break;
		default:
			if (dht.readData(true))
			{
				float ftemp = dht.ConvertTemperature(CELCIUS) + dpt9ToFloat(config->Offset) * 0.01f;
				int16_t temp = (int16_t)(ftemp * 100);
				uint16_t hum = (uint16_t)(dht._lastHumidity * 100);

				// Werte auf Plausibilität prüfen
				// ändern nur wenn Abweichung zum alten Wert < 10%
				// oder wenn mehrmals nacheinander die Abweichung größer war
				if (
						(
							((int16_t)BCU->comObjects->objectReadFloat(firstComIndex + 1) < temp * 1.1F)
							&&
							((int16_t)BCU->comObjects->objectReadFloat(firstComIndex + 1) > temp * 0.9F)
							&&
							((int16_t)BCU->comObjects->objectReadFloat(firstComIndex + 3) < hum * 1.1F)
							&&
							((int16_t)BCU->comObjects->objectReadFloat(firstComIndex + 3) > hum * 0.9F)
						)
						||
						retries > 2
					)
				{
					BCU->comObjects->objectWriteFloat(firstComIndex + 1, temp);
					BCU->comObjects->objectWrite(firstComIndex + 2, (byte*)&ftemp);
					BCU->comObjects->objectWriteFloat(firstComIndex + 3, hum);
					retries = 0;
				}
				else
				{
					retries++;
				}
			}
			if (config->PowerPin != 99)
			{
				digitalWrite(ARMPinItem::PortPins[config->PowerPin], false);
			}

			nextAction = now + (config->Delay * 1000) - 2000;
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
