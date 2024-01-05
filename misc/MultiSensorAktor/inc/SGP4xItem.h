/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef SGP4XITEM_H_
#define SGP4XITEM_H_

#include <sblib/eib/bcu_base.h>
#include <GenericItem.h>
#include <TempHumSensorConfig.h>
#include <sblib/i2c/SGP4x.h>

class SGP4xItem : public GenericItem
{
public:
	SGP4xItem(byte firstComIndex, TempHumSensorConfig* config, GenericItem* nextItem, uint16_t& objRamPtr);
	~SGP4xItem() = default;

	void Loop(uint32_t now, int updatedObjectNo);
	int ConfigLength() { return sizeof(TempHumSensorConfig); }
	int ComObjCount() { return 4; }

protected:
	TempHumSensorConfig *config;
	SGP4xClass sgp4x;
	uint32_t nextAction = 0;
	byte state = 0;
	bool configured = false;
	uint16_t baseline = 0;
};

#endif /* SGP4XITEM_H_ */
