/*
 * app_raincenter.h
 *
 *  Created on: 11.09.2020
 *      Author: Darthyson
 */

#ifndef APP_RAINCENTER_H_
#define APP_RAINCENTER_H_

#include <sblib/types.h>

/**
 * A communication object was updated from somebody via the EIB bus.
 *
 * @param objno - the ID of the communication object.
 */
void objectUpdated(int objno);

/**
 * The value of an input channel changed.
 *
 * @param channel - the input channel (0..)
 * @param value - the new value
 */
void inputChanged(int channel, int value);

/**
 * Handle the periodic functions.
 */
void checkPeriodic(void);

/**
 * Initialization of the application
 */
void initApplication(void);



#endif /* APP_RAINCENTER_H_ */
