/*
 * in16.h
 *
 *  Created on: 17.07.2014
 *      Author: glueck
 */

#ifndef IN16_H_
#define IN16_H_

typedef struct
{
    unsigned int inputs;
} In8TestState;

extern In8TestState _refState;
extern In8TestState _stepState;

void _gatherState (In8TestState * state, In8TestState * refState);
void _loop(In8TestState * refState);

void _input1Set(In8TestState * refState);
void _input1Clear(In8TestState * refState);
void _input2Set(In8TestState * refState);
void _input2Clear(In8TestState * refState);
void _input3Set(In8TestState * refState);
void _input3Clear(In8TestState * refState);
void _input4Set(In8TestState * refState);
void _input4Clear(In8TestState * refState);
void _input5Set(In8TestState * refState);
void _input5Clear(In8TestState * refState);
void _input6Set(In8TestState * refState);
void _input6Clear(In8TestState * refState);
void _resetInputs(void);

#endif /* IN16_H_ */
