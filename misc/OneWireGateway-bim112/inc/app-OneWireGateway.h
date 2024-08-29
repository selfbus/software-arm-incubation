/*
 *  OneWire Gateway Applikation for LPC1115
 *
 *  Copyright (C) 2021-2024 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef ONE_WIRE_GATEWAY_BIM112_H_
#define ONE_WIRE_GATEWAY_BIM112_H_

#include <sblib/core.h>

/**
 * Called from the main loop whenever a com object has been updated
 * via a group address write
 */
void objectUpdated(int objno);

/**
 * Called during each iteration of the main loop
 */
void checkPeriodicFuntions(void);

/**
 * Called during the initialization of the application
 */
void initApplication(void);

void searchForNewOneWireDevices(void);

void sendTemperatureObject(uint8_t onewireDeviceCount);

#endif /* ONE_WIRE_GATEWAY_BIM112_H_ */
