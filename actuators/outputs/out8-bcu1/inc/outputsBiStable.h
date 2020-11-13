/*
 *  outputsBoStable.cpp - Handle the update of the port pins in accordance to the
 *                need of the application
 *
 *  Copyright (C) 2014-2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef OUTPUTSBISTABLE_H_
#define OUTPUTSBISTABLE_H_

#include "outputs.h"

class OutputsBiStable : public Outputs
{
public:
	OutputsBiStable() : Outputs() {};

    virtual void updateOutputs(void);
    virtual void checkPWM(void);
};

#ifdef BI_STABLE
extern OutputsBiStable relays;
#endif

#endif /* OUTPUTSBISTABLE_H_ */
