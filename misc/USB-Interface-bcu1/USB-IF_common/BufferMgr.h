/*
 *  BufferMgr.h - Manages a number of fixed size buffers
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef BUFFERMGR_H_
#define BUFFERMGR_H_

#include <stdint.h>

#define BUFF_CNT 8
#define BUFF_SIZE 68

class BufferMgr
{
public:
    BufferMgr(void);
    void Purge(void);
    int AllocBuffer(void);
    int FreeBuffer(int no);
    uint8_t* buffptr(int no);
protected:
    uint8_t data[BUFF_CNT][BUFF_SIZE] = {0};
    bool alloctable[BUFF_CNT] = {false};
};

extern BufferMgr buffmgr;

#endif /* BUFFERMGR_H_ */
