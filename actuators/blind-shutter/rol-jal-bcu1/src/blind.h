/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  Handle the blind (Raffstore) mode of an channel.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef BLIND_H_
#define BLIND_H_

#include <channel.h>

class Blind: public Channel
{
public:
    Blind(unsigned int number);
    virtual void startUp(void);
    virtual void startDown(void);
    virtual void stop(void);
    virtual void periodic(void);

protected:
    unsigned int slatTime;          //!< time the slats need from fully closed to fully open
    unsigned int slatPosition;      //!< current position of the slats
    unsigned int slatStartPosition; //!< current position of the slats
};

#endif /* BLIND_H_ */
