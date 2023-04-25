/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef SHT4XITEM_H_
#define SHT4XITEM_H_

#include <sblib/eib/bcu_base.h>
#include <GenericItem.h>
#include <TempHumSensorConfig.h>
#include <sblib/i2c/SHT4x.h>

class SHT4xItem : public GenericItem
{
public:
	SHT4xItem(BcuBase* bcu, byte firstComIndex, TempHumSensorConfig* config, GenericItem* nextItem, uint16_t& objRamPtr);
	~SHT4xItem() = default;

	void Loop(uint32_t now, int updatedObjectNo);
	int ConfigLength() { return sizeof(TempHumSensorConfig); }
	int ComObjCount() { return 4; }

protected:
	TempHumSensorConfig *config;
    SHT4xClass sht4x;
	uint32_t nextAction = 0;
	byte state = 0;
	float offset;
};

#endif /* SHT2XITEM_H_ */
