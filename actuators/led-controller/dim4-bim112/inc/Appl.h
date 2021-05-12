/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *                2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef appl_h
#define appl_h

#include <sblib/types.h>

/**
 * A communication object was updated from somebody via the EIB bus.
 *
 * @param objno - the ID of the communication object.
 */
void objectUpdated(int objno);

/**
 * Handle the periodic functions.
 */
void checkPeriodic(void);

/**
 * Initialization of the application
 */
void initApplication(void);

/**
 * Handle the different Objects
 */
//void handleSwitchObject(unsigned int objectValue, int channel);
void handleRelDimmingObject(int objectValue, int channel);
void handleAbsDimmingObject(int objectValue, int channel);
void handleRGBAbsDimmingObject(int objectValue);
void handleHSVAbsDimmingObject(int objectValue);
void handleBlocking1Object(int objectValue, int channel);
void handleBlocking2Object(int objectValue, int channel);

/**
 * check if Switchstatus or Dimmvalue to send
 */
void handleBusReturn(int channel);

/**
 * Channel reacts of central Objects
 */
bool centralObjectsActive(int channel);

#endif /*appl_h*/
