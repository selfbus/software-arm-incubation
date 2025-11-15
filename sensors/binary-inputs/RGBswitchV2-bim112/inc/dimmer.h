/*
 *  dimmer.h - 
 *
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef DIMMER_H_
#define DIMMER_H_

#include "channel.h"
#include "switch.h"
/*
 *
 */
class Dimmer: public _Switch_
{
public:
    Dimmer(unsigned int no, unsigned int longPress, unsigned int channelConfig,
            unsigned int busReturn, unsigned int value);
    virtual void inputChanged(int value);
    virtual void checkPeriodic(void);
private:
    unsigned int oneButtonDimmer;
    unsigned int upDownInverse;
    unsigned int stepsDownWidthDimmer;
    unsigned int stepsUpWidthDimmer;
    unsigned int repeatDimmerTime;
    unsigned int doStopFlag;
    int onOffComObjNo;
    int dimValComObjNo;
    int stateComObjNo;
};

#endif /* DIMMER_H_ */
