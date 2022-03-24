/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <PCA9555DItem.h>
#include <sblib/i2c.h>

extern int portPins[32];

PCA9555DItem::PCA9555DItem(BcuBase* bcu, byte firstComIndex, PCA9555DConfig *config, GenericItem* nextItem) : GenericItem(bcu, firstComIndex, nextItem), config(config)
{
	IRQItem *lastIRQItem = nullptr;
	byte* currentConfig = (byte*)config + sizeof(PCA9555DConfig);
	byte nextComObj = firstComIndex;

	outFlag = 0;
	lockFlag = 0;
	inOutConfig = 0;
	checkTime = 0;
	forceCheckTill = 0;

	if (config->IntPin != 99)
	{
		pinMode(portPins[config->IntPin], INPUT | PULL_UP);
	}

	for (byte i = 0; i < 16; i++)
	{
		if (config->PinEnabled & (1 << i))
		{
			PCA9555DPinConfig* pinConfig = (PCA9555DPinConfig*)currentConfig;
			switch (pinConfig->Type)
			{
			case PCA9555DPinTypeInput:
				{
					pins[i] = new InputPin(bcu, nextComObj, &(pinConfig->Input));
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
				pins[i] = new OutputPin(bcu, nextComObj, &pinConfig->Output);
				break;
			case PCA9555DPinTypePWM:
				{
					PWMPin* pwmPin = new PWMPin(bcu, nextComObj, &pinConfig->PWM, this);
					lastIRQItem = pwmPin;
					pins[i] = pwmPin;
				}
				break;
			default:
				inOutConfig |= (1 << i);
				continue;
			}
			bcu->comObjects->requestObjectRead(firstComIndex + 2);
			configured = true;

			nextComObj += 3;
			currentConfig += sizeof(PCA9555DPinConfig);
		}
		else
		{
			pins[i] = nullptr;
		}
	}

	if (configured)
	{
		initPCA();
		forceCheckTill = millis() + checkTime;
		if (lastIRQItem != nullptr)
		{
			this->nextPin = lastIRQItem->nextPin;
			lastIRQItem->nextPin = this;
			useIRQ = true;
		}
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
	if (newState != outFlag && !useIRQ)
	{
		outFlag = newState;
		uint16_t output = outFlag | irqStatus;
		unsigned char tempBytes1[] = { 0x02, (byte)(output), (byte)(output >> 8) };
		Chip_I2C_MasterSend(I2C0, 0x20 + config->Address, tempBytes1 , 3);
	}

	if (config->IntPin == 99 || !digitalRead(portPins[config->IntPin]))
	{
		forceCheckTill = now + checkTime;
	}

	if (forceCheckTill > now)
	{
		if (!useIRQ)
		{
			if (Chip_I2C_MasterCmdRead(I2C0, 0x20 | config->Address, 0, (byte*)&inputStatus, 2) != 2)
			{
				return;
			}
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

void PCA9555DItem::Irq(void* item, byte newValue)
{
/*	for (int i = 0; i < 16; i++)
	{
		if (pins[i] == item)
		{
			irqStatus &= ~(1 << i);
			irqStatus |= newValue << i;
		}
	}*/
}


void PCA9555DItem::Irq(uint32_t now, uint16_t timerVal)
{
/*	uint16_t output = outFlag | irqStatus;
	unsigned char tempBytes1[] = { 0x02, (byte)(output), (byte)(output >> 8) };
	Chip_I2C_MasterSend(I2C0, 0x20 + config->Address, tempBytes1 , 3);
	if (!timerVal)
	{
		Chip_I2C_MasterCmdRead(I2C0, 0x20 | config->Address, 0, (byte*)&inputStatus, 2);
	}*/
}
