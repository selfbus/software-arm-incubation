/*
 *  channel.h - Base class for the different channel operations
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <sblib/types.h>
#include <sblib/timeout.h>

/*
 * Base class which defines the interface used by the different channel
 * types for the real implementation.
 *
 * Common code also resides in this class.
 */
class Channel
{
public:
    Channel (unsigned int no) : locked(0), number(no) { };
    virtual void inputChanged(int value, int longPress);
    virtual void checkPeriodic(void);
    virtual void setLock(unsigned int value);
    bool locked;

protected:
    unsigned int number;
    Timeout  timeout;
};

#endif /* CHANNEL_H_ */
