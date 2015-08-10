/*
 *  jalo.h - 
 *
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef JALO_H_
#define JALO_H_

#include "channel.h"
#include "switch.h"
/*
 *
 */
class Jalo: public _Switch_
{
public:
    Jalo(unsigned int no, unsigned int longPress, unsigned int channelConfig,
            unsigned int busReturn, unsigned int value);
    virtual void inputChanged(int value);
    virtual void checkPeriodic(void);
private:
    unsigned int oneButtonShutter;
    unsigned int shortLongInverse;
    unsigned int upDownInverse;
    int upDownComObjNo;
    int stopComObjNo;
    int directionComObjNo;
};

#endif /* JALO_H_ */
