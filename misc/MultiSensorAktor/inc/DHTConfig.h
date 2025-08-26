/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef DHTCONFIG_H_
#define DHTCONFIG_H_

#include <sblib/eib/bcu_base.h>

#include <TempHumSensorConfig.h>

#pragma pack(1)
struct DHTConfig : TempHumSensorConfig
{
	uint8_t PowerPin;
};
#pragma pack()


#endif /* DHTCONFIG_H_ */
