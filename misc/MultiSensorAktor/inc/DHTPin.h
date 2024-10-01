/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef DHTITEM_H_
#define DHTITEM_H_

#include <sblib/eib/bcu_base.h>
#include <sblib/sensors/dht.h>
#include <GenericPin.h>
#include <TempHumSensorConfig.h>

class DHTPin : public GenericPin
{
public:
	DHTPin(int port, byte firstComIndex, TempHumSensorConfig* config, bool dht11, uint16_t& objRamPointer);
	~DHTPin() = default;

	byte GetState(uint32_t now, byte updatedObjectNo);

	int ConfigLength() { return sizeof(TempHumSensorConfig); }
	int ComObjCount() { return 4; }

protected:
	uint32_t nextAction = 0;
	byte state = 0;
	DHT dht;
	TempHumSensorConfig* config;
	byte retries;
};


#endif /* DHTITEM_H_ */
