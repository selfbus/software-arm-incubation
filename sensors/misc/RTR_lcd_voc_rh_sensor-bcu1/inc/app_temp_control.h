/*
 *  Copyright (c) 2016 Oliver Stefan <os252@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef app_temp_control_h
#define app_temp_control_h

unsigned int factortime_to_ms(unsigned int startaddress);
void rebootApplication(void);
void objectUpdated(int objno);
void initApplication(void);
void handlePeriodic(void);
void inputChanged(int channel, int val, uint8_t pressType);

// value to store if the Application Board is connected to the controller
extern bool applicationBoardConnected;

#endif
