/*
 *  Copyright (c) 2014-2015 Stefan Taferner <stefan.taferner@gmx.at>
 *                          Martin Glueck <martin@mangari.org>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef DIMENCODER_H_
#define DIMENCODER_H_

void dimEncChannelChanged(int channel, int pinValue);
void dimEncPeriod(int channel);
void dimEncoderSetup(int channel);
void dimEncoderLock(int state, int channel);

#endif /* DIMENCODER_H_ */
