/*
 *  out8.cpp - Common functions for the tests of the out8
 *
 *  Copyright (C) 2014-2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "protocol.h"
#include "out8.h"
#include "app_out8.h"
#include "catch.hpp"
#include "sblib/timer.h"

Out8TestState _refState;
Out8TestState _stepState;

void _clearOutputs(void)
{
    for (int i=0; i < 8; i++)
    {
        digitalWrite(outputPins[i], 0);
    }
}

void _gatherState (Out8TestState * state, Out8TestState * refState)
{
    int i;
    state->outputs = 0;
    state->pwm_on  = timer16_0.match(MAT2) != 857;
    for (i=0; i < 8; i++)
    {
        state->outputs |= (digitalRead(outputPins[i]) << i);
    }
    if (refState)
    {
        REQUIRE(state->outputs == refState->outputs);
        REQUIRE(state->pwm_on  == refState->pwm_on);
    }
}

void _loop(Out8TestState * refState)
{
    bcu.loop();
    loop();
}

void _output1Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x01;
    refState->pwm_on   = 0;
}
void _output2Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x02;
    refState->pwm_on   = 0;
}
void _output3Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x04;
    refState->pwm_on   = 0;
}
void _output4Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x08;
    refState->pwm_on   = 0;
}
void _output5Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x10;
    refState->pwm_on   = 0;
}
void _output6Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x20;
    refState->pwm_on   = 0;
}
void _output7Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x40;
    refState->pwm_on   = 0;
}
void _output8Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x80;
    refState->pwm_on   = 0;
}

void _output34Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs |= 0x0C;
    refState->pwm_on   = 0;
}

void _enablePWM(Out8TestState * refState)
{
    _loop(refState);
    refState->pwm_on = 0x01;
}

void _output1Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x01;
}
void _output2Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x02;
}
void _output3Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x04;
}
void _output4Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x08;
}
void _output5Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x10;
}
void _output6Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x20;
}
void _output7Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x40;
}
void _output8Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x80;
}

void _output34Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs &= ~0x0C;
}
