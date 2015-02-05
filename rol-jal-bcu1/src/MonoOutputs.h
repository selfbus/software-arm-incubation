/*
 *  MonoOutputs.h - 
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef MONOOUTPUTS_H_
#define MONOOUTPUTS_H_

#include "outputs.h"
#include <sblib/timeout.h>
/*
 *
 */
class MonoOutputs: public Outputs
{
public:
    MonoOutputs(void);
private:
    Timeout _pwmTimeout;
};

#if OUTPUTTYPE == MONO
extern MonoOutputs outputs;
#endif

#endif /* MONOOUTPUTS_H_ */
