/*
 *  Copyright (c) 2014-2015 Stefan Taferner <stefan.taferner@gmx.at>
 *                          Martin Glueck <martin@mangari.org>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef SWITCH_H_
#define SWITCH_H_
/*
 * The value of an input channel of type "switch" changed.
 *
 * @param channel - the input channel (0..7)
 * @param pinValue - the current value of the input pin (0 or 1)
 */
void switchChannelChanged(int channel, int pinValue);

void switchPeriodic (int channel);

void switchSetup(int channel);

void switchLock(int state, int channel);

#endif /* SWITCH_H_ */
