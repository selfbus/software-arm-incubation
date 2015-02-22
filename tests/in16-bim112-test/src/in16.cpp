/*
 *  in16.cpp - Intrastructure for the test cases
 *
 *  Copyright (c) 2014 Martin Glück <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "protocol.h"
#include "in16.h"
//#include "app_in8.h"
#include "catch.hpp"
#include "sblib/timer.h"

// allow access to the inputState member
#define protected public
#include "input.h"
#undef protected

extern Input inputs;

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
    if (value)
    {
        inputs.inputState |= mask;
        refState->inputs |=  mask;
    }
    else
    {
        inputs.inputState &= ~mask;
        refState->inputs &= ~mask;
    }
    _loop(refState);
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

void _input7Set(In8TestState * refState)
{
	_inputSet (6, 1, refState);
}
void _input7Clear(In8TestState * refState)
{
	_inputSet (6, 0, refState);
}

void _input8Set(In8TestState * refState)
{
	_inputSet (7, 1, refState);
}
void _input8Clear(In8TestState * refState)
{
	_inputSet (7, 0, refState);
}

void _input9Set(In8TestState * refState)
{
	_inputSet (8, 1, refState);
}
void _input9Clear(In8TestState * refState)
{
	_inputSet (8, 0, refState);
}

void _input10Set(In8TestState * refState)
{
	_inputSet (9, 1, refState);
}
void _input10Clear(In8TestState * refState)
{
	_inputSet (9, 0, refState);
}

void _input11Set(In8TestState * refState)
{
	_inputSet (10, 1, refState);
}
void _input11Clear(In8TestState * refState)
{
	_inputSet (10, 0, refState);
}

void _input12Set(In8TestState * refState)
{
	_inputSet (11, 1, refState);
}
void _input12Clear(In8TestState * refState)
{
	_inputSet (11, 0, refState);
}

void _input13Set(In8TestState * refState)
{
	_inputSet (12, 1, refState);
}
void _input13Clear(In8TestState * refState)
{
	_inputSet (12, 0, refState);
}

void _input14Set(In8TestState * refState)
{
	_inputSet (13, 1, refState);
}
void _input14Clear(In8TestState * refState)
{
	_inputSet (13, 0, refState);
}

void _input15Set(In8TestState * refState)
{
	_inputSet (14, 1, refState);
}
void _input15Clear(In8TestState * refState)
{
	_inputSet (14, 0, refState);
}

void _input16Set(In8TestState * refState)
{
	_inputSet (15, 1, refState);
}
void _input16Clear(In8TestState * refState)
{
	_inputSet (15, 0, refState);
}


void _resetInputs(void)
{
	for (int i = 0; i < 8; i++)
	    //digitalWrite(inputPins [i], 0);
	    ;
}
