/*
 *  Copyright (c) 2014-2015 Stefan Taferner <stefan.taferner@gmx.at>
 *                          Martin Glueck <martin@mangari.org>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef JALO_H_
#define JALO_H_

void jaloChannelChanged(int channel, int pinValue);
void jaloPeriod(int channel);
void jaloSetup(int channel);
void jaloLock(int state, int channel);

#endif /* JALO_H_ */
