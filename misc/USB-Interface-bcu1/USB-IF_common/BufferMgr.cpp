/*
 *  BufferMgr.cpp - Manages a number of fixed size buffers
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <stdio.h>
#include "BufferMgr.h"
#include "error_handler.h"

BufferMgr buffmgr;

BufferMgr::BufferMgr(void)
{
	Purge();
}

void BufferMgr::Purge(void)
{
	for (int i=0; i < BUFF_CNT; i++)
		alloctable[i] = false;
}

int BufferMgr::AllocBuffer(void)
{
	for (int i=0; i < BUFF_CNT; i++)
	{
		if (!alloctable[i])
		{
			alloctable[i] = true;
			return i;
		}
	}
	//failHardInDebug(); ///\todo remove on release
	return -1;
}

int BufferMgr::FreeBuffer(int no)
{
	if ((no < 0) || (no >= BUFF_CNT))
		return -1;
	if (alloctable[no])
	{
		alloctable[no] = false;
		return 0;
	}
	return -1;
}

uint8_t* BufferMgr::buffptr(int no)
{
	if ((no < 0) || (no >= BUFF_CNT))
		return nullptr;
	return &data[no][0];
}
