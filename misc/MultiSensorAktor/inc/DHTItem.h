/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef DHTITEM_H_
#define DHTITEM_H_

#include <sblib/eib/bcu_base.h>
#include <sblib/sensors/dht.h>
#include <GenericItem.h>
#include <PortConfig.h>

class DHTItem : public GenericItem
{
public:
	DHTItem(BcuBase* bcu, int port, int firstComIndex, PortConfig* config, GenericItem* nextItem);
	~DHTItem() = default;

	virtual void Loop(int updatedObjectNo);

protected:
	unsigned int nextAction = 0;
	byte state = 0;
	DHT dht;
	float offset;
};


#endif /* DHTITEM_H_ */
