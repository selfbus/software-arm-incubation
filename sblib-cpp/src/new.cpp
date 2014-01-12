/*
 *  new.cpp - C++ memory handling.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/utils.h>

/*
 * When this function gets called, an object was created that has a
 * pure virtual function.
 *
 * E.g. the class Stream (in stream.h) has the function flush():
 * virtual void flush() = 0;
 *
 * If this function is not implemented in the subclass, you end up here.
 */
extern "C" void __cxa_pure_virtual()
{
    fatalError();
}
