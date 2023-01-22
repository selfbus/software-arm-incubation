/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <ARMPinItem.h>
#include <sblib/eibMASK0701.h>
#include <sblib/spi.h>
#include <sblib/i2c.h>
#include <cstring>
#include <DeviceConfig.h>
#include <ARMPinConfig.h>
#include <PCA9555DConfig.h>
#include <CCS811Config.h>
#include <GenericItem.h>
#include <PCA9555DItem.h>
#include <CCS811Item.h>
#include <ARMPinItem.h>
#include <SHT2xItem.h>
#include <HelperFunctions.h>

APP_VERSION("MSA     ", "0", "10");
#define CONFIG_ADDRESS 0x4800

MASK0701 bcu = MASK0701();

// Digital pin for LED
#define PIO_LED PIO2_0

GenericItem *firstItem = nullptr;

const unsigned char hardwareVersion[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x48 };

MemMapper memMapper = MemMapper(0xe000, 0x1000, false);

/**
 * Application setup
 */
BcuBase* setup()
{
    bcu.begin(0x13A, 0x01, 0x02);


    memcpy(bcu.userEeprom->order(), hardwareVersion,
            sizeof(hardwareVersion));

    pinMode(PIO_LED, OUTPUT);
    digitalWrite(PIO_LED, 1);

    memMapper.addRange(0x6000, 0xF00);
    bcu.setMemMapper(&memMapper);

    HelperFunctions::setFlagsTablePtr(&bcu, 0x6C5);
    uint16_t objRamPointer = 0x5FC;

    DeviceConfig* deviceConfig = (DeviceConfig*)&(*bcu.userEeprom)[CONFIG_ADDRESS];
    byte nextComObj = 1;
    byte* configPos = memMapper.memoryPtr(0x6000, false);

    if (deviceConfig->BusSwitches & BusSwitchSPI0)
    {
    	SPI spi0 = SPI(SPI_PORT_0);
    	spi0.setClockDivider(120); // 400kHz
    	spi0.setDataSize(SPI_DATA_8BIT);
    	spi0.begin();

    }

    if (deviceConfig->BusSwitches & BusSwitchSPI1)
    {
    	SPI spi0 = SPI(SPI_PORT_1);
    	spi0.setClockDivider(120); // 400kHz
    	spi0.setDataSize(SPI_DATA_8BIT);
    	spi0.begin();

    }

    if (deviceConfig->BusSwitches & BusSwitchI2C)
    {
    	i2c_lpcopen_init();
    	Chip_I2C_SetClockRate(I2C0, 400000);

    	for (int i = 0; i < deviceConfig->PCA9555DCount; i++)
    	{
    		firstItem = new PCA9555DItem(&bcu, nextComObj, (PCA9555DConfig*)configPos, firstItem, objRamPointer);
    		nextComObj += firstItem->ComObjCount();
    		configPos += firstItem->ConfigLength();
    	}

    	for (int i = 0; i < deviceConfig->CCS811Count; i++)
    	{
    		firstItem = new CCS811Item(&bcu, nextComObj, (CCS811Config*)configPos, firstItem, objRamPointer);
    		nextComObj += firstItem->ComObjCount();
    		configPos += firstItem->ConfigLength();
    	}

    	if (deviceConfig->SHT2xCount)
    	{
    		firstItem = new SHT2xItem(&bcu, nextComObj, (SHT2xConfig*)configPos, firstItem, objRamPointer);
    		nextComObj += firstItem->ComObjCount();
    		configPos += firstItem->ConfigLength();
    	}
    }


    configPos = &(*bcu.userEeprom)[CONFIG_ADDRESS + sizeof(DeviceConfig)];
	for (int i = 0; i < 32; i++)
	{
		if (deviceConfig->PortAssignment[i] > 0 && deviceConfig->PortAssignment[i] <= 32)
		{
			firstItem = new ARMPinItem(&bcu, nextComObj, deviceConfig->PortAssignment[i] - 1, (ARMPinConfig*)configPos, firstItem, objRamPointer);
			nextComObj += firstItem->ComObjCount();
			configPos += firstItem->ConfigLength();
		}
	}

    interrupts();

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
    	uint32_t now = millis();
        GenericItem* item = firstItem;
        while (item != nullptr)
        {
        	item->Loop(now, objNo);
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

/**
 * The processing loop while no KNX-application is loaded
 */
void loop_noapp()
{

}
