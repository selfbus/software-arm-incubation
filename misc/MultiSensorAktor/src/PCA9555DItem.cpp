/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/bcu_base.h>
#include <sblib/i2c.h>

#include <PCA9555DItem.h>
#include <DummyPin.h>
#include <ARMPinItem.h>
#include <InputPin.h>
#include <OutputPin.h>
#include <PWMPin.h>

PCA9555DItem::PCA9555DItem(byte firstComIndex, PCA9555DConfig *config, GenericItem* nextItem, uint16_t& objRamPointer) : GenericItem(firstComIndex, nextItem), config(config)
{
	byte* currentConfig = (byte*)config + sizeof(PCA9555DConfig);
	byte nextComObj = firstComIndex;

	outFlag = 0;
	//lockFlag = 0;
	inOutConfig = 0;
	checkTime = 0;
	forceCheckTill = 0;

	for (byte i = 0; i < 16; i++)
	{
		if (config->PinEnabled & (1 << i))
		{
			PCA9555DPinConfig* pinConfig = (PCA9555DPinConfig*)currentConfig;
			switch (pinConfig->Type)
			{
			case PCA9555DPinTypeInput:
				{
					pins[i] = new InputPin(nextComObj, &pinConfig->Input, objRamPointer);
					inOutConfig |= (1 << i);
					if (pinConfig->Input.DebounceTime > checkTime)
					{
						checkTime = pinConfig->Input.DebounceTime;
					}
					if (pinConfig->Input.LongTime > checkTime)
					{
						checkTime = pinConfig->Input.LongTime;
					}
					checkTime += 10;
				}
				break;
			case PCA9555DPinTypeOutput:
				pins[i] = new OutputPin(nextComObj, &pinConfig->Output, objRamPointer);
				break;
			case PCA9555DPinTypePWM:
				{
					PWMPin* pwmPin = new PWMPin(nextComObj, &pinConfig->PWM, this, objRamPointer);
					pins[i] = pwmPin;
				}
				break;
			default:
				pins[i] = new DummyPin();
				inOutConfig |= (1 << i);
				continue;
			}
			BCU->comObjects->requestObjectRead(firstComIndex + 2);
			configured = true;

			nextComObj += 4;
			currentConfig += sizeof(PCA9555DPinConfig);
		}
		else
		{
			pins[i] = new DummyPin();
		}
	}

	if (configured)
	{
		initPCA();
		forceCheckTill = millis() + checkTime;
	}

	configLength = currentConfig - (byte*)config;
	comObjCount = nextComObj - firstComIndex;
};

void PCA9555DItem::initPCA()
{
	uint8_t bytesTransferred = 0;
 	unsigned char tempBytes1[] = { 0x02, (byte)(outFlag), (byte)(outFlag >> 8) };
	bytesTransferred += Chip_I2C_MasterSend(I2C0, 0x20 | config->Address, tempBytes1 , 3);
	unsigned char tempBytes2[] = { 0x04, 0X00, 0x00 };
	bytesTransferred += Chip_I2C_MasterSend(I2C0, 0x20 | config->Address, tempBytes2 , 3);
	unsigned char tempBytes3[] = { 0x06, (byte)inOutConfig, (byte)(inOutConfig >> 8) };
	bytesTransferred += Chip_I2C_MasterSend(I2C0, 0x20 | config->Address, tempBytes3, 3);

	if (bytesTransferred != 9)
	{
		configured = false;
	}
}

void PCA9555DItem::Loop(uint32_t now, int updatedObjectNo)
{
	if (!configured)
	{
		return;
	}

	uint16_t newState = outFlag;
	for (int i = 0; i < 16; i++)
	{
		if (pins[i] != nullptr)
		{
			newState &= ~(1 << i);
			newState |= pins[i]->GetState(now, updatedObjectNo) << i;
		}
	}
	if (newState != outFlag)
	{
		outFlag = newState;
		uint16_t output = outFlag;
		unsigned char tempBytes1[] = { 0x02, (byte)(output), (byte)(output >> 8) };
		Chip_I2C_MasterSend(I2C0, 0x20 + config->Address, tempBytes1 , 3);
	}

	if (config->IntPin == 99 || (config->IntPin >= 0 && config->IntPin <=32 && !digitalRead(ARMPinItem::PortPins[config->IntPin])))
	{
		forceCheckTill = now + checkTime;
	}

	if (forceCheckTill > now)
	{
		uint16_t inputStatus = 0;
		if (Chip_I2C_MasterCmdRead(I2C0, 0x20 | config->Address, 0, (byte*)&inputStatus, 2) != 2)
		{
			return;
		}

		for (int i = 0; i < 16; i++)
		{
			if (pins[i] != nullptr)
			{
				pins[i]->PutValue(now, inputStatus & (1 << i));
			}
		}
	}
}
