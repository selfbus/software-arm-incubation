/*
 *  switch.h - 
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef SWITCH_H_
#define SWITCH_H_

#include "channel.h"

/*
 *
 */
class Switch: public Channel
{
public:
    Switch(unsigned int no, word config, unsigned int  channelConfig);
    virtual void inputChanged(int value, int longPress);
    virtual void checkPeriodic(void);
    virtual void setLock(unsigned int value);
private:
    unsigned int shortLongOperation;
    unsigned int action;
    unsigned int usage_rising;
    unsigned int usage_falling;
    unsigned int delay;
};

#endif /* SWITCH_H_ */
