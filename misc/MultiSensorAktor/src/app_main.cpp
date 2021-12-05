/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eibMASK0701.h>
#include <cstring>
#include <DeviceConfig.h>
#include <PortConfig.h>
#include <GenericItem.h>
#include <DHTItem.h>
#include <OutputItem.h>
#include <InputItem.h>
#include <PWMItem.h>


#define CONFIG_ADDRESS 0x4700

MASK0701 bcu = MASK0701();

// Digital pin for LED
#define PIO_LED PIO2_0

GenericItem *firstItem = nullptr;
int portPins[32] = {
		PIO0_2, PIO0_3, PIO0_4, PIO0_5, PIO0_6, PIO0_7, PIO0_8, PIO0_9, PIO0_11,
		PIO1_0, PIO1_1, PIO1_2, PIO1_5, PIO1_10,
		PIO2_0, PIO2_1, PIO2_2, PIO2_3, PIO2_4, PIO2_5, PIO2_6, PIO2_7, PIO2_8, PIO2_9, PIO2_10, PIO2_11,
		PIO3_0, PIO3_1, PIO3_2, PIO3_3, PIO3_4, PIO3_5
};

const unsigned char hardwareVersion[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x48 };
/**
 * Application setup
 */
BcuBase* setup()
{
    bcu.begin(0x13A, 0x02, 1);

    memcpy(bcu.userEeprom->order(), hardwareVersion,
            sizeof(hardwareVersion));

    pinMode(PIO_LED, OUTPUT);
    digitalWrite(PIO_LED, 1);

    DeviceConfig* deviceConfig = (DeviceConfig*)&(*bcu.userEeprom)[CONFIG_ADDRESS];

    for (int i = 0; i < NUM_PORTS; i++)
    {
    	if (deviceConfig->PortsEnable & (1u << i))
    	{
			PortConfig* config = &deviceConfig->ports[i];
			switch (config->Type)
			{
			case PortTypeInputFloating:
			case PortTypeInputPullup:
			case PortTypeInputPulldown:
				firstItem = new InputItem(&bcu, portPins[i], (i * 4) + 1, config, firstItem);
				break;
			case PortTypeOutput:
				firstItem = new OutputItem(&bcu, portPins[i], (i * 4) + 1, config, firstItem);
				break;
			case PortTypeDHT11:
			case PortTypeDHT22:
				firstItem = new DHTItem(&bcu, portPins[i], (i * 4) + 1, config, firstItem);
				break;
			case PortTypePWM:
				firstItem = new PWMItem(&bcu, portPins[i], (i * 4) + 1, config, firstItem);
				break;
			}
    	}
    }

    return &bcu;
}

/**
 * The application's main.
 */
void loop()
{
    // Handle updated communication objects
    int objNo = -1;

    do
    {
        GenericItem* item = firstItem;
        while (item != nullptr)
        {
        	item->Loop(objNo);
        	item = item->GetNextItem();
        }
    }
    while ((objNo = bcu.comObjects->nextUpdatedObject()) >= 0);

    // Sleep up to 1 millisecond if there is nothing to do
    if (bcu.bus->idle())
    {
        waitForInterrupt();
    }
}
