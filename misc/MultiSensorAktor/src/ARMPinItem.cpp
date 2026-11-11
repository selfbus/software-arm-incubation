/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/bcu_base.h>
#include <sblib/digital_pin.h>

#include <ARMPinItem.h>
#include <DHTPin.h>
#include <InputPin.h>
#include <OutputPin.h>
#include <CounterPin.h>
#include <PWMPin.h>
#include <PwmController.h>
#include <DummyPin.h>

const constexpr int ARMPinItem::PortPins[36];

ARMPinItem::ARMPinItem(uint8_t firstComIndex, uint8_t portNumber, ARMPinConfig *config, GenericItem* nextItem, uint16_t& objRamPointer) : GenericItem(firstComIndex, nextItem), config(config)
{
	port = PortPins[portNumber];
	switch (config->BaseConfig.Type)
	{
	case ARMPinType::InputFloating:
		pinMode(port, INPUT);
		pin = new InputPin(firstComIndex, &config->Input, objRamPointer);
		break;
	case ARMPinType::InputPullup:
		pinMode(port, INPUT | PULL_UP);
		pin = new InputPin(firstComIndex, &config->Input, objRamPointer);
		break;
	case ARMPinType::InputPulldown:
		pinMode(port, INPUT | PULL_DOWN);
		pin = new InputPin(firstComIndex, &config->Input, objRamPointer);
		break;
	case ARMPinType::Output:
		pinMode(port, OUTPUT);
		digitalWrite(port, !config->Output.Invert);
		pin = new OutputPin(firstComIndex, &config->Output, objRamPointer);
		break;
	case ARMPinType::PWM:
		// output at the inactive level, hardware PWM if the pin has a free timer match output, else in software
		pin = new PWMPin(firstComIndex, &config->PWM, this, objRamPointer,
		                 pwmController.attach(port, PWMPin::inverted(config->PWM)));
		break;
	case ARMPinType::DHT11:
		pin = new DHTPin(port, firstComIndex, &config->DHT, true, objRamPointer);
		break;
	case ARMPinType::DHT22:
		pin = new DHTPin(port, firstComIndex, &config->DHT, false, objRamPointer);
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
		pin->GetState(now, updatedObjectNo); // hands a new value to pwmController
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
	return sizeof(ARMPinConfig);
}

int ARMPinItem::ComObjCount()
{
	return 4;
}

