/*
 * LedIndication.h
 *
 *  Created on: 04.07.2015
 *      Author: glueck
 */

#ifndef LEDINDICATION_H_
#define LEDINDICATION_H_

#include <sblib/spi.h>
#include "config.h"

class Led_Indication
{
public:
	Led_Indication(int spiPort, int pinData, int pinClock, int pinRCK, int pinOE);
	void begin(void);
	void setStatus(unsigned int ledNo, unsigned int state, bool updated = true);
    void updateLeds(void);

protected:
	SPI spi;
    unsigned int pinRCK;
    unsigned int pinOE;
	unsigned int _state;
};


#endif /* LEDINDICATION_H_ */
