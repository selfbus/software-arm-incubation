/*
 *  counter.h - 
 *
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef COUNTER_H_
#define COUNTER_H_

#include "channel.h"
#include "switch.h"
/*
 *
 */
class Counter: public _Switch_
{
public:
    Counter(unsigned int no, unsigned int longPress, unsigned int channelConfig,
            unsigned int busReturn, unsigned int value);
    virtual void inputChanged(int value);
    virtual void checkPeriodic(void);
private:
    unsigned int modeCounter;
    unsigned int txDiffCounter;
    int counterComObjNo;
    int resetComObjNo;

};

#endif /* COUNTER_H_ */
