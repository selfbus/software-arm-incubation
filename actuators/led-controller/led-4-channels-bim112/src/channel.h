/*
 *  channel.h - Base class for a channel of the LED actuator
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef CHANNEL_H_
#define CHANNEL_H_

class Channel
{
public:
    Channel();

    bool isOn();

    void setValue(unsigned char value);

private:
             int  number;
    unsigned char pwm;
};


inline bool Channel::isOn()
{
    return pwm > 0;
}

#endif /* CHANNEL_H_ */
