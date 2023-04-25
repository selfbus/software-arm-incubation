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
#include <DummyPin.h>

const int ARMPinItem::PortPins[32] = {
		PIO0_2, PIO0_3, PIO0_4, PIO0_5, PIO0_6, PIO0_7, PIO0_8, PIO0_9, PIO0_11,
		PIO1_0, PIO1_1, PIO1_2, PIO1_5, PIO1_10,
		PIO2_0, PIO2_1, PIO2_2, PIO2_3, PIO2_4, PIO2_5, PIO2_6, PIO2_7, PIO2_8, PIO2_9, PIO2_10, PIO2_11,
		PIO3_0, PIO3_1, PIO3_2, PIO3_3, PIO3_4, PIO3_5
};

ARMPinItem::ARMPinItem(BcuBase* bcu, byte firstComIndex, byte portNumber, ARMPinConfig *config, GenericItem* nextItem, uint16_t& objRamPointer) : GenericItem(bcu, firstComIndex, nextItem), config(config)
{
	port = PortPins[portNumber];
	switch (config->BaseConfig.Type)
	{
	case ARMPinType::InputFloating:
		pinMode(port, INPUT);
		pin = new InputPin(bcu, firstComIndex, &config->Input, objRamPointer);
		break;
	case ARMPinType::InputPullup:
		pinMode(port, INPUT | PULL_UP);
		pin = new InputPin(bcu, firstComIndex, &config->Input, objRamPointer);
		break;
	case ARMPinType::InputPulldown:
		pinMode(port, INPUT | PULL_DOWN);
		pin = new InputPin(bcu, firstComIndex, &config->Input, objRamPointer);
		break;
	case ARMPinType::Output:
		pinMode(port, OUTPUT);
		digitalWrite(port, !config->Output.Invert);
		pin = new OutputPin(bcu, firstComIndex, &config->Output, objRamPointer);
		break;
	case ARMPinType::PWM:
		pinMode(port, OUTPUT);
		digitalWrite(port, 0);
		pin = new PWMPin(bcu, firstComIndex, &config->PWM, this, objRamPointer);
		break;
	case ARMPinType::DHT11:
		pin = new DHTPin(bcu, port, firstComIndex, &config->DHT, true, objRamPointer);
		break;
	case ARMPinType::DHT22:
		pin = new DHTPin(bcu, port, firstComIndex, &config->DHT, false, objRamPointer);
		break;
	default:
		pin = new DummyPin();
		break;
	}
};

void ARMPinItem::Loop(uint32_t now, int updatedObjectNo)
{
	switch (config->BaseConfig.Type)
	{
	case ARMPinType::InputFloating:
	case ARMPinType::InputPullup:
	case ARMPinType::InputPulldown:
		pin->PutValue(now, digitalRead(port));
		break;
	case ARMPinType::Output:
		digitalWrite(port, pin->GetState(now, updatedObjectNo));
		break;
	case ARMPinType::PWM:
		//digitalWrite(port, pin->GetState(now, updatedObjectNo));
		pin->GetState(now, updatedObjectNo);
		break;
	case ARMPinType::DHT11:
	case ARMPinType::DHT22:
		pin->GetState(now, updatedObjectNo);
		break;
	default:
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
