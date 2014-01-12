/*
 *  printable.h - Interface for classes that can be printed.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_printable_h
#define sblib_printable_h

class Print;

/**
 * The Printable class provides a way for classes to allow themselves to be printed.
 *
 * By deriving from Printable and implementing the printTo method, it is possible
 * for users to print out instances of this class by passing them into the usual
 * Print::print and Print::println methods.
 */
class Printable
{
public:
    /**
     * Print the object to the print target.
     *
     * @param out - the target to print to.
     * @return The number of bytes written.
     */
    virtual int printTo(Print& out) const = 0;
};

#endif /*sblib_printable_h*/
