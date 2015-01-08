/*
 *  in8.cpp - Intrastructure for the test cases
 *
 *  Copyright (c) 2014 Martin Glück <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "protocol.h"
#include "in8.h"
#include "app_in8.h"
#include "catch.hpp"
#include "sblib/timer.h"

In8TestState _refState;
In8TestState _stepState;

void _gatherState (In8TestState * state, In8TestState * refState)
{
}

void _loop(In8TestState * refState)
{
    bcu.loop();
    loop();
}


void _inputSet(unsigned int channel, unsigned int value, In8TestState * refState)
{
	unsigned int mask = 1 << channel;
	digitalWrite(inputPins [channel], value);
    _loop(refState);
    if (value) refState->inputs |=  mask;
    else       refState->inputs &= ~mask;
}

void _input1Set(In8TestState * refState)
{
	_inputSet (0, 1, refState);
}
void _input1Clear(In8TestState * refState)
{
	_inputSet (0, 0, refState);
}

void _input2Set(In8TestState * refState)
{
	_inputSet (1, 1, refState);
}
void _input2Clear(In8TestState * refState)
{
	_inputSet (1, 0, refState);
}
void _input3Set(In8TestState * refState)
{
	_inputSet (2, 1, refState);
}
void _input3Clear(In8TestState * refState)
{
	_inputSet (2, 0, refState);
}
void _input4Set(In8TestState * refState)
{
	_inputSet (3, 1, refState);
}
void _input4Clear(In8TestState * refState)
{
	_inputSet (3, 0, refState);
}

void _input5Set(In8TestState * refState)
{
	_inputSet (4, 1, refState);
}
void _input5Clear(In8TestState * refState)
{
	_inputSet (4, 0, refState);
}

void _input6Set(In8TestState * refState)
{
	_inputSet (5, 1, refState);
}
void _input6Clear(In8TestState * refState)
{
	_inputSet (5, 0, refState);
}

void _resetInputs(void)
{
	for (int i = 0; i < 8; i++)
	    digitalWrite(inputPins [i], 0);
}
