/*
 *  out8.cpp - Common functions for the tests of the out8
 *
 *  Copyright (C) 2014-2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef OUT8_H_
#define OUT8_H_

typedef struct
{
    unsigned int outputs;
    unsigned int pwm_on;
} Out8TestState;

extern Out8TestState _refState;
extern Out8TestState _stepState;

void _gatherState (Out8TestState * state, Out8TestState * refState);
void _loop(Out8TestState * refState);
void _output1Set(Out8TestState * refState);
void _output2Set(Out8TestState * refState);
void _output3Set(Out8TestState * refState);
void _output4Set(Out8TestState * refState);
void _output5Set(Out8TestState * refState);
void _output6Set(Out8TestState * refState);
void _output7Set(Out8TestState * refState);
void _output8Set(Out8TestState * refState);
void _output34Set(Out8TestState * refState);
void _enablePWM(Out8TestState * refState);
void _output1Clear(Out8TestState * refState);
void _output2Clear(Out8TestState * refState);
void _output3Clear(Out8TestState * refState);
void _output4Clear(Out8TestState * refState);
void _output5Clear(Out8TestState * refState);
void _output6Clear(Out8TestState * refState);
void _output7Clear(Out8TestState * refState);
void _output8Clear(Out8TestState * refState);
void _output34Clear(Out8TestState * refState);

void _clearOutputs(void);

#endif /* OUT8_H_ */
