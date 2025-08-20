/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef HOERMANN_H_
#define HOERMANN_H_

#include <sblib/types.h>
#include "HoermannState.h"
#include "CRC.h"


#define DCTRL 4

class Hoermann {
public:
	Hoermann();

	void loop();
	void open();
	void close();
	void stop();
	void venting();
	void light(bool state);
	void emergencyStop(bool state);

	HoermannState state;

protected:
	uint8_t slaveResponse[2] = { 0x00, 0x10 };
	int stateMachine = 0;
	int address = 0;
	int counter = 0;
	int length = 0;
	int position = 0;
	byte data[15];
	uint8_t myCounter = 0;
	CRC crc;

	void sendResponse(uint8_t addr, uint8_t bytes[], uint8_t len);
};

#endif /* HOERMANN_H_ */
