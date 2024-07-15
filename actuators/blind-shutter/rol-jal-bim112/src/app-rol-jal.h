#ifndef ROL_JAL_BIM112_SRC_APP_ROL_JAL_H_
#define ROL_JAL_BIM112_SRC_APP_ROL_JAL_H_
/*
 *  app-rol-jal.cpp - The application for the 4 channel blinds/shutter actuator
 *  acting as a MDT JAL-0410.01 (ver. 2.8)
 *
 *  Copyright (C) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "channel.h"

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
void initApplication(short channelPositions[] = nullptr, short channelSlatPositions[] = nullptr);

/**
 * Called if there was a Bus voltage failure
 */
void stopApplication();

/**
 * Get the actual State of each channel
 */
void getChannelPositions(short channelPositions[], short channelSlatPositions[]);


#endif /* ROL_JAL_BIM112_SRC_APP_ROL_JAL_H_ */
