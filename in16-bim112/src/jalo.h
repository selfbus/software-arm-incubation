/*
 *  jalo.h - 
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef JALO_H_
#define JALO_H_

#include "channel.h"

/*
 *
 */
class Jalo: public Channel
{
public:
    Jalo(unsigned int no);
    virtual void inputChanged(int value);
};

#endif /* JALO_H_ */
