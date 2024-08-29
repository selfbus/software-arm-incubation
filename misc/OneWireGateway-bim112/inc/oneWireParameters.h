/*
 *  OneWire Gateway Applikation for LPC1115
 *
 *  Copyright (C) 2021-2024 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#ifndef ONEWIREPARAMETERS_H_
#define ONEWIREPARAMETERS_H_

#include <sblib/core.h>

bool getParasiteMode(uint8_t channel);
bool getUnknownAddressCyclicSend(uint8_t channel);
void getOneWireAddress(uint8_t oneWireAddress[], uint8_t channel, uint8_t deviceNumber);
bool checkCyclicSend(uint8_t channel, uint8_t deviceNumber);
bool checkValueChangeSend(uint8_t channel, uint8_t deviceNumber);
uint8_t getCyclicTime(uint8_t channel, uint8_t deviceNumber);
float getChangeValue(uint8_t channel, uint8_t deviceNumber);
float getCorrectionValue(uint8_t channel, uint8_t deviceNumber);
void turnOneWireAddress(uint8_t originalAddress[], uint8_t turnedAddress[]);

#endif /* ONEWIREPARAMETERS_H_ */
