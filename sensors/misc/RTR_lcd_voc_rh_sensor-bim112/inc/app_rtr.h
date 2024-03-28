/*
 *  Copyright (c) 2016-2021 Oliver Stefan
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef app_temp_control_h
#define app_temp_control_h

unsigned int factortime_to_ms(uint8_t timeParaNumber);
void rebootApplication(void);
void objectUpdated(int objno);
void sendTargetTemperature();
void initApplication(void);
float getChangeValue(uint8_t timeParaNumber);
void handlePeriodic(void);
void inputChanged(int channel, int val, uint8_t pressType);

// value to store if the Application Board is connected to the controller
extern bool applicationBoardConnected;

// true if external target temperature can be used
// in this case in the lcd menu has to be an entry to set the target temperature source back to external
extern bool use_external_target_temperature;

#endif
