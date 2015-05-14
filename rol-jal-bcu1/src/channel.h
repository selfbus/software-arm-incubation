/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_
#include <sblib/types.h>

#define NO_OF_CHANNELS 4
#define NO_OF_OUTPUTS  (NO_OF_CHANNELS * 2)

extern const int outputPins[NO_OF_OUTPUTS];

class Channel
{
public:
    enum Direction { UP, DOWN, SLAT_OPEN, SLAT_CLOSE, STOP };

    Channel(unsigned int number);
    virtual void startUp(void)   = 0;
    virtual void startDown(void) = 0;
    virtual void stop(void)      = 0;
    virtual void periodic(void)  = 0;

protected:
    unsigned int timeToPercentage(unsigned int startTime, unsigned int maxTime);

    unsigned int number;
    unsigned int openTime;       //!< time the channel needs from fully closed to fully open
    unsigned int closeTime;      //!< time the channel needs from fully open   to fully closed
             int position;       //!< current channel position
             int startPosition;  //!< position when the movement started
    unsigned int startTime;
    Direction    status;
};

#endif /* CHANNEL_H_ */
