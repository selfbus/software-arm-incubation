/*
 *  Copyright (c) 2014-2015 Stefan Taferner <stefan.taferner@gmx.at>
 *                          Martin Glueck <martin@mangari.org>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef DIMMER_H_
#define DIMMER_H_

void dimChannelChanged(int channel, int pinValue);
void dimPeriod(int channel);
void dimSetup(int channel);
void dimLock(int state, int channel);

#endif /* DIMMER_H_ */
