/*
 *  outputs.h -
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef OUTPUTS_H_
#define OUTPUTS_H_

#include "config.h"

class Outputs
{
public:
    Outputs(void);
    void init(void);

protected:
#ifdef ZERO_DETECT
    int _state;
    int _port_0_set;
    int _port_0_clr;
    int _port_2_set
    int _port_2_clr;
#endif
#ifdef HAND_ACTUATION
    int _handCount;
#endif
};

#endif // OUTPUTS_H_
