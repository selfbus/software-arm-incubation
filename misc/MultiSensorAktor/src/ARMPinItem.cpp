/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <ARMPinItem.h>
#include <sblib/digital_pin.h>
#include <InputPin.h>
#include <OutputPin.h>
#include <CounterPin.h>
#include <PWMPin.h>
#include <DHTPin.h>

ARMPinItem::ARMPinItem(BcuBase* bcu, int port, byte firstComIndex, ARMPinConfig *config, GenericItem* nextItem) : GenericItem(bcu, firstComIndex, nextItem), config(config), port(port)
{
	switch (config->BaseConfig.Type)
	{
	case ARMPinTypeInputFloating:
		pinMode(port, INPUT);
		pin = new InputPin(bcu, firstComIndex, &config->Input);
		break;
	case ARMPinTypeInputPullup:
		pinMode(port, INPUT | PULL_UP);
		pin = new InputPin(bcu, firstComIndex, &config->Input);
		break;
	case ARMPinTypeInputPulldown:
		pinMode(port, INPUT | PULL_DOWN);
		pin = new InputPin(bcu, firstComIndex, &config->Input);
		break;
	case ARMPinTypeOutput:
		pinMode(port, OUTPUT);
		digitalWrite(port, !config->Output.Invert);
		pin = new OutputPin(bcu, firstComIndex, &config->Output);
		break;
	case ARMPinTypePWM:
		pinMode(port, OUTPUT);
		digitalWrite(port, 0);
		pin = new PWMPin(bcu, firstComIndex, &config->PWM, this);
		break;
	case ARMPinTypeDHT11:
		pin = new DHTPin(bcu, port, firstComIndex, &config->DHT, true);
		break;
	case ARMPinTypeDHT22:
		pin = new DHTPin(bcu, port, firstComIndex, &config->DHT, false);
		break;
	}
 };

void ARMPinItem::Loop(uint32_t now, int updatedObjectNo)
{
	switch (config->BaseConfig.Type)
	{
	case ARMPinTypeInputFloating:
	case ARMPinTypeInputPullup:
	case ARMPinTypeInputPulldown:
		pin->PutValue(now, digitalRead(port));
		break;
	case ARMPinTypeOutput:
		digitalWrite(port, pin->GetState(now, updatedObjectNo));
		break;
	case ARMPinTypePWM:
		//digitalWrite(port, pin->GetState(now, updatedObjectNo));
		pin->GetState(now, updatedObjectNo);
		break;
	case ARMPinTypeDHT11:
	case ARMPinTypeDHT22:
		pin->GetState(now, updatedObjectNo);
		break;
	}
}

int ARMPinItem::ConfigLength()
{
	return sizeof(config->BaseConfig) + pin->ConfigLength();
}

int ARMPinItem::ComObjCount()
{
	return pin->ComObjCount();
}

void ARMPinItem::Irq(void* item, byte value)
{
	digitalWrite(port, value ^ config->PWM.Invert);
}
