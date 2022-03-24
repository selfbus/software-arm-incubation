/*
 * CCS811Config.h
 *
 *  Created on: 17.12.2021
 *      Author: dridders
 */

#ifndef CCS811CONFIG_H_
#define CCS811CONFIG_H_

#pragma pack(1)
struct CCS811Config
{
	uint16_t PreFan;
	uint16_t PreMeasure;
	uint16_t Delay;
	byte addr;
	byte wakePin;
};
#pragma pack()


#endif /* CCS811CONFIG_H_ */
