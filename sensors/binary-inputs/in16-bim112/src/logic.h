/*
 * logic.h
 *
 *  Created on: 08.08.2015
 *      Author: Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 */

#ifndef LOGIC_H_
#define LOGIC_H_
#include <sblib/types.h>
#include <sblib/timeout.h>
#include "debug.h"
#include "input.h"

#define MAX_LOGIC 4

/*
 * Logic handler
 *
 */

#define LOGIC_OP_OR 0
#define LOGIC_OP_AND 1
#define LOGIC_OP_DISABLED 255

#define SEND_COND_NONE   0
#define SEND_COND_INPUT  1
#define SEND_COND_OUTPUT 2
#define SEND_COND_SCENE  3

#define CHAN_MODE_DISABLED 0
#define CHAN_MODE_NORMAL   1
#define CHAN_MODE_INVERTED 2

class Logic
{
public:
    Logic (unsigned int  logicBase, unsigned int no, unsigned int chans, unsigned int busreturn);
    void inputChanged(int num, int value);
    void objectUpdated(int objno);

private:
    void doLogic(void);

    unsigned int configBase;
    unsigned int number;
    unsigned int channels;

    unsigned int logicOperation; // @0
    unsigned int sendCondition;  // @1 1:inp, 2:outp, 3:scene
    unsigned int outputInverted; // @2
    unsigned int numScene;
    unsigned int inputCfgPtr;
    unsigned int inputs[MAX_CHANNELS+2];

    int extLogicalObjectAComObjNo;
    int extLogicalObjectBComObjNo;
	int outLogicalObjectComObjNo;
};
#endif /* LOGIC_H_ */
