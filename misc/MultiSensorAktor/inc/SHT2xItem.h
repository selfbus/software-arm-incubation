/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef SHT2XITEM_H_
#define SHT2XITEM_H_

#include <sblib/eib/bcu_base.h>
#include <sblib/i2c/SHT2x.h>

#include <GenericItem.h>
#include <TempHumSensorConfig.h>

class SHT2xItem : public GenericItem
{
public:
	SHT2xItem(byte firstComIndex, TempHumSensorConfig* config, GenericItem* nextItem, uint16_t& objRamPtr);
	~SHT2xItem() = default;

	void Loop(uint32_t now, int updatedObjectNo);
	int ConfigLength() { return sizeof(TempHumSensorConfig); }
	int ComObjCount() { return 4; }

protected:
	TempHumSensorConfig *config;
    SHT2xClass sht2x;
	uint32_t nextAction = 0;
	byte state = 0;
	float offset;
};

#endif /* SHT2XITEM_H_ */
