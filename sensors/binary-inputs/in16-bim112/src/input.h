/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef INPUT_H_
#define INPUT_H_

#include <sblib/debounce.h>
#include <sblib/timeout.h>

#define MAX_CHANNELS 16

class Input
{
public:
    void begin(int noOfChannels, int baseAddress);
    virtual void scan(void);
    bool checkInput(unsigned int channel, unsigned int * value);
protected:
    unsigned int noOfChannels;
    unsigned int debounceTime;
    unsigned int inputState;
    Debouncer inputDebouncer[16];
};

#endif /* INPUT_H_ */
