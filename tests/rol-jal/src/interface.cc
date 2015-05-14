/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/digital_pin.h>
#define protected public
#include "sblib/timeout.h"
#include "channel.h"
#include "shutter.h"
#include "blind.h"
#include "hand_actuation.h"
#undef protected
#include "app-rol-jal.h"

typedef struct
{
    unsigned char  type;
    unsigned char  state;
    unsigned char  direction;
    unsigned char  positionValid;
    unsigned short features;
    unsigned int   limits;
    unsigned int   moveForTime;
    unsigned int   startTime;
             short position;         //!< current channel position
             short startPosition;    //!< position when the movement started
             short targetPosition;   //!< requested target position
             short savedPosition;
             short slatPosition;       //!< current position of the slats
             short slatStartPosition;  //!< current position of the slats
             short slatTargetPosition; //!< requested target position
             short slatSavedPosition;
    unsigned int   timeout;
} ChannelInfo;
#define COPY(i, ch, f) i->f = ch->f

extern "C" unsigned int accessIO(unsigned int pin, int value)
{
    if (value == -1)
    {   // read the current value
        if (pin & 0x100)
            return digitalRead(handPins[pin & 0xFF]);
        return digitalRead(outputPins[pin & 0xFF]);
    }
    else
    {   // write the value
        if (pin & 0x100)
        {
            unsigned int mask = 1 << (pin & 0xFF);
            if (value) handAct._inputState |=  mask;
            else       handAct._inputState &= ~mask;
        }
        else digitalWrite(outputPins[pin & 0xFF], value);
    }
    return -1;
}

extern "C" void queryChannel(unsigned    int no, ChannelInfo * info)
{
    Channel * ch = channels[no];
    if (ch)
    {
        info->type = ch->channelType();
        COPY(info, ch, state);
        COPY(info, ch, direction);
        COPY(info, ch, positionValid);
        COPY(info, ch, features);
        COPY(info, ch, limits);
        COPY(info, ch, moveForTime);
        COPY(info, ch, startTime);
        COPY(info, ch, features);
        COPY(info, ch, position);
        COPY(info, ch, startPosition);
        COPY(info, ch, targetPosition);
        COPY(info, ch, savedPosition);
        info->timeout = ch->timeout.timeout;
        if (ch->channelType() == Channel::BLIND)
        {
            Blind * obj = (Blind *) ch;
            COPY(info, obj, slatPosition);
            COPY(info, obj, slatStartPosition);
            COPY(info, obj, slatTargetPosition);
            COPY(info, obj, slatSavedPosition);
        }
    }
}
