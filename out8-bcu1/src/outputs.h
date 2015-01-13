/*
 *  app_out8.cpp - The application for the 8 channel output acting as a Jung 2118
 *
 *  Copyright (C) 2014-2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef _outputs_h_
#define _outputs_h_ 1

#include "app_out8.h"
#include <sblib/timeout.h>
#include <sblib/timer.h>
#include <sblib/digital_pin.h>

#define PWM_TIMEOUT 50
#define PWM_PERIOD     857
#define PWM_DUTY_33    (588)


class Outputs
{
public:
    Outputs() : _relayState(0)
              , _prevRelayState(0)
              , _inverted(0)
              , _blocked(0)
              {};

    void begin(unsigned int initial, unsigned int inverted);
    unsigned int pendingChanges(void);
    unsigned int channel(unsigned int channel);
    void updateChannel(unsigned int channel, unsigned int value);
    void setChannel(unsigned int channel);
    void clearChannel(unsigned int channel);
    unsigned int toggleChannel(unsigned int channel);
    unsigned int blocked(unsigned int channel);
    void setBlocked(unsigned int channel);
    void clearBlocked(unsigned int channel);
    void checkPWM(void);
    void updateOutputs(void);
protected:
    unsigned int _relayState;
    unsigned int _prevRelayState;
    unsigned int _inverted;
    unsigned int _modified;
    unsigned int _blocked;
    Timeout      _pwm_timeout;
};

inline void Outputs::begin (unsigned int initial, unsigned int inverted)
{
    pinMode(PIO3_2, OUTPUT_MATCH);  // configure digital pin PIO3_2(PWM) to match MAT2 of timer16 #0
    //pinMode(PIO1_4, OUTPUT);
    timer16_0.begin();

    timer16_0.prescaler((SystemCoreClock / 10000000) - 1);
    timer16_0.matchMode(MAT2, SET);  // set the output of PIO3_2 to 1 when the timer matches MAT1
    timer16_0.match(MAT2, 0);        // match MAT1 when the timer reaches this value
    timer16_0.pwmEnable(MAT2);       // enable PWM for match channel MAT1

    // Reset the timer when the timer matches MAT3
    timer16_0.matchMode(MAT3, RESET);
    timer16_0.match(MAT3, PWM_PERIOD);     // match MAT3 ato create 14lHz
    timer16_0.start();
    _pwm_timeout.start(PWM_TIMEOUT); // start the timer to switch back to a PWM operation
    //digitalWrite(PIO1_4, 1);
    _relayState     = initial;
    _prevRelayState = ~initial;
    _inverted       = inverted;
}

unsigned int Outputs::pendingChanges(void)
{
    return _relayState ^ _prevRelayState;
}

unsigned int Outputs::channel(unsigned int channel)
{
    return _relayState & (1 << channel) ? 1 : 0;
}

void Outputs::updateChannel(unsigned int channel, unsigned int value)
{
    if (value) setChannel(channel);
    else       clearChannel(channel);
}

void Outputs::setChannel(unsigned int channel)
{
    unsigned int mask = 1 << channel;
    if (! (_blocked & mask))
       _relayState |=  mask;
}

void Outputs::clearChannel(unsigned int channel)
{
    unsigned int mask = 1 << channel;
    if (! (_blocked & mask))
        _relayState &= ~mask;
}

unsigned int Outputs::toggleChannel(unsigned int channel)
{
    if (this->channel(channel))
    {
        clearChannel(channel);
        return 0;
    }
    else
    {
        setChannel(channel);
        return 1;
    }
}

unsigned int Outputs::blocked(unsigned int channel)
{
    return _blocked & (1 << channel);
}

void Outputs::setBlocked(unsigned int channel)
{
    _blocked |= (1 << channel);
}

void Outputs::clearBlocked(unsigned int channel)
{
    _blocked &= ~(1 << channel);
}

void Outputs::checkPWM(void)
{
    if(_pwm_timeout.expired())
    {
        timer16_0.match(MAT2, PWM_DUTY_33);
        //digitalWrite(PIO1_4, 0);
    }
}

void Outputs::updateOutputs(void)
{
    unsigned int mask = 0x01;
    unsigned int i;
    unsigned int state = _relayState ^ _inverted;
    unsigned int prevState = _prevRelayState ^  _inverted;
    if ((state ^ prevState) & state)
    {   // at least one outputs needs to be switched ON -> disable
        // the PWM
        timer16_0.match(MAT2, 0);// disable the PWM
        _pwm_timeout.start(PWM_TIMEOUT);
    }
    for(i = 0; i < NO_OF_CHANNELS; i++, mask <<= 1)
    {
        digitalWrite(outputPins[i], state & mask);
    }
    _prevRelayState = _relayState;
}

#endif
