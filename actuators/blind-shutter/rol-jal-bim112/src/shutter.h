/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  Handle the shutter (== Rolladen) mode of a channel.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef ROL_JAL_BIM112_SRC_SHUTTER_H_
#define ROL_JAL_BIM112_SRC_SHUTTER_H_

#include <channel.h>

class Shutter: public Channel
{
public:
    Shutter() = delete;
    Shutter(unsigned int number, unsigned int address, short position);
    virtual ~Shutter() = default;
};

#endif /* ROL_JAL_BIM112_SRC_SHUTTER_H_ */
