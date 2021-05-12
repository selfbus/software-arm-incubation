/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  Handle the shutter (== Rolladen) mode of a channel.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef SHUTTER_H_
#define SHUTTER_H_

#include <channel.h>

class Shutter: public Channel
{
public:
    Shutter(unsigned int number);
    virtual void startUp(void);
    virtual void startDown(void);
    virtual void stop(void);
    virtual void periodic(void);
};

#endif /* SHUTTER_H_ */
