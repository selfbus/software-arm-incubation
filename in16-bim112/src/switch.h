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
class _Switch_: public Channel
{
protected:
	_Switch_(unsigned int no) : Channel(no) {};
public:
    virtual void setLock(unsigned int value);
};

class Switch: public _Switch_
{
public:
    Switch(unsigned int no, unsigned int  channelConfig, unsigned int busReturn, unsigned int value);
    virtual void inputChanged(int value, int longPress);
    virtual void checkPeriodic(void);
private:
    unsigned int action;
    unsigned int usage_rising;
    unsigned int usage_falling;
    unsigned int delay;
};

class Switch2Level : public _Switch_
{
public:
	Switch2Level(unsigned int no, unsigned int  channelConfig, unsigned int busReturn, unsigned int value);
    virtual void inputChanged(int value, int longPress);
private:
    bool lastWasLong;
    unsigned int shortAction;
    unsigned int longAction;
    unsigned int usage_rising;
    unsigned int usage_falling;
};

#endif /* SWITCH_H_ */
