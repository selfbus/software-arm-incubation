/*
 *  channel.cpp - Base class for the different channel operations
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "channel.h"

void Channel::setLock(unsigned int value)
{
    locked = value;
}

void Channel::inputChanged(int value, int longPress)
{

}

void Channel::checkPeriodic(void)
{

}
